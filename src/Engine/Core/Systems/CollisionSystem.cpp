#include "CollisionSystem.h"
#include <algorithm>
#include <cmath>
#include "../Components/CollisionComponent.h"
#include "../GameObjectBase.h"
#include "../../../Utils/Math.h"

namespace ETG
{
    void CollisionSystem::Update()
    {
        CollectActive();
        DetectContacts();
        DispatchEnterAndStay();
        DispatchExits();
        Commit();
    }

    void CollisionSystem::ForgetComponent(const CollisionComponent* component)
    {
        //Blanked, never erased, for the same reason SafeRegistry blanks: a phase below may be standing at an index
        //in one of these lists right now, and erasing would shift everything after it out from under that index.
        //Both dispatch phases step over the blanks
        for (CollisionComponent*& active : Active)
        {
            if (active == component) active = nullptr;
        }

        for (Contact& contact : Contacts)
        {
            if (contact.Self == component || contact.Other == component)
            {
                contact.Self = nullptr;
                contact.Other = nullptr;
            }
        }
    }

    //PHASE 1.
    void CollisionSystem::CollectActive()
    {
        Active.clear();

        CollisionComponent::GetAllCollRegistries().ForEach([](CollisionComponent* component)
        {
            if (!component->IsCollisionEnabled() || !component->Owner) return;

            Active.push_back(component);
        });

        
        for (CollisionComponent* component : Active)
        {
            //Every collider's bounds are refreshed here, in one pass, before a single test runs below. This is the
            //whole point of the split - every test reads positions from the same instant, so it cannot matter
            //whose Update moved them first
            component->UpdateBounds();
            component->StillColliding.Clear();
        }
    }

    //PHASE 2. Every pair, exactly once. Reads only - no events, so nothing can change underneath us
    void CollisionSystem::DetectContacts()
    {
        Contacts.clear();

        //j starts one past i, so a pair is visited once instead of twice. The old scheme had no choice about that:
        //every component walked the entire registry on its own behalf, so (a,b) and (b,a) were separate sweeps
        for (size_t i = 0; i < Active.size(); ++i)
        {
            CollisionComponent* a = Active[i];

            for (size_t j = i + 1; j < Active.size(); ++j)
            {
                CollisionComponent* b = Active[j];

                //> “A objesi, B’nin layer’ıyla gerçekleşen collision event’lerini dinliyor mu?”
                //“A’nın maskesinde B’nin layer’ı var mı, yani A, B’yi dinliyor mu?” islemi bu
                //
                const bool aWatchesB = (a->Mask & b->Layer) != 0;
                const bool bWatchesA = (b->Mask & a->Layer) != 0;
                if (!aWatchesB && !bWatchesA) continue;

                //The overlap comes back from the same test that decided whether there is one, so the impact point
                //is four arithmetic ops on a rect already in hand instead of a second full intersection
                ETG::FloatRect overlap;
                if (!a->CheckCollision(b, overlap)) continue;

                const ETG::Vector2f impactPoint = overlap.getCenter();

                //Both sides learn about the contact from this one test, which is what makes the result symmetric.
                //Under the old scheme each side discovered it separately, on its own schedule, or not at all
                if (aWatchesB)
                {
                    a->StillColliding.Add(b);
                    Contacts.push_back({a, b, impactPoint});
                }

                if (bWatchesA)
                {
                    b->StillColliding.Add(a);
                    Contacts.push_back({b, a, impactPoint});
                }
            }
        }
    }

    //PHASE 3. The contact list is final before the first listener runs, so what game code does in response cannot
    //change who is reported as touching whom this frame
    void CollisionSystem::DispatchEnterAndStay()
    {
        //Indexed, and size() re-read every step, for the reason SafeRegistry::ForEach is: a listener may destroy an
        //object, and ForgetComponent blanks the contacts naming it while we are standing in this list
        for (size_t i = 0; i < Contacts.size(); ++i)
        {
            //Copied out, because ForgetComponent may blank this very slot from inside the broadcast below
            const Contact contact = Contacts[i];
            if (!contact.Self || !contact.Other) continue;
            if (!contact.Self->Owner || !contact.Other->Owner) continue;

            //A listener earlier in this same dispatch may have switched this side off - a projectile does exactly
            //that the moment it lands. It has already been told its contacts ended, so it hears no more of them.
            //Note that the OTHER side is deliberately not checked: the overlap really happened, and whether the
            //other side has since switched itself off is a matter of who came first in this list. Skipping on that
            //would hand the ordering problem straight back
            if (!contact.Self->IsCollisionEnabled()) continue;

            const CollisionEventData eventData(contact.Self->Owner, contact.Other->Owner, contact.Other, contact.ImpactPoint);

            //Enter or Stay is decided against last frame's record, which Commit has not overwritten yet
            if (contact.Self->PrevFrameCollisions.Contains(contact.Other))
                contact.Self->OnCollisionStay.Broadcast(eventData);
            else
                contact.Self->OnCollisionEnter.Broadcast(eventData);
        }
    }

    //PHASE 4. Who were we touching last frame but are not any more?
    void CollisionSystem::DispatchExits()
    {
        for (size_t i = 0; i < Active.size(); ++i)
        {
            CollisionComponent* component = Active[i];
            if (!component || !component->Owner) continue;

            //Comparing against StillColliding rather than re-testing the bounds catches strictly more: a collider
            //that stopped overlapping, but equally one that switched itself off this frame, or one whose Mask no
            //longer names the other. None of them made it into StillColliding, so all of them read as gone
            component->PrevFrameCollisions.ForEach([component](CollisionComponent* other)
            {
                if (!other->Owner || component->StillColliding.Contains(other)) return;

                //The two no longer overlap, so there is no impact point to report
                const CollisionEventData eventData(component->Owner, other->Owner, other, ETG::Vector2f{0, 0});
                component->OnCollisionExit.Broadcast(eventData);
            });
        }
    }

  //   - Collision aktif olmalı.
  // - Owner bulunmalı.
  // - Layer, verilen solidMask içinde olmalı.
  // - Hareket alanıyla kesişmeli.Bu da yanlizca karakterin Duvarla overlap oldugu zaman gerceklesiyor 
    void CollisionSystem::CollectSolids(const ETG::FloatRect& area, const uint32_t solidMask, const CollisionComponent* ignore)
    {
        Solids.clear();

        CollisionComponent::GetAllCollRegistries().ForEach([&area, solidMask, ignore](const CollisionComponent* component)
        {
            if (component == ignore) return;
            if (!component->IsCollisionEnabled() || !component->Owner) return;

            //NOTE: Burasi cok onemli Component’ın layer’ı solid maskesiyle eşleşmiyorsa fonksiyondan çık.
            //SolidMask ile &'in 0 cikabilmesi icin Layer'in Obstacle veya bir ustu olmasi sart. 
            if ((component->Layer & solidMask) == 0) return;

            //Okunuyor, tazelenmiyor. Bounds herkes icin frame'de bir kez CollectActive'de hesaplaniyor; katı olan
            //tek sey icin bu yeterli: duvar hareket etmez, dolayisiyla gecen frame'in kutusuyla bu frame'in
            //kutusu aynidir. Kendi basina HAREKET EDEN bir kati cisim olsaydi bir frame geriden gelirdi
            const ETG::FloatRect bounds = component->GetCollisionBounds();
            if (!area.intersects(bounds)) return;

            //NOTE: Karakter duvarla collide olursa sadece bu calisacak 
            Solids.push_back(bounds);
        });
    }

    //Bu fonksiyonu anlamadim ve en sonunda vazgeçtim
    CollisionSystem::SlideResult CollisionSystem::MoveAndSlide(CollisionComponent* body, const ETG::Vector2f& delta, const ETG::Vector2f& velocity)
    {
        constexpr float ContactSkin = 0.01f;
        
        SlideResult result{delta, velocity, false};

        if (!body || !body->Owner) return result;
        if (body->BlockingMask == CollisionLayer::None) return result;
        if (delta.x == 0.f && delta.y == 0.f) return result;

        //Burada guvenilmiyor, yeniden hesaplaniyor; cunku bu kod frame'in ORTASINDA calisiyor ve sahibi bu tick
        //icinde baska bir sey tarafindan zaten bir kez hareket ettirilmis olabilir - yurumeden once cozulen bir
        //knockback, ya da bir dash. CollectActive'in tazelemesine yaslanmak yanlis olur: o, butun bunlardan
        //SONRA calisiyor
        body->UpdateBounds();
        const ETG::FloatRect box = body->GetCollisionBounds();
        if (box.width <= 0.f || box.height <= 0.f) return result;

        //sweep = box ile box + delta'yı birlikte içine alan en küçük dikdörtgen. Yani "şu an neredeyim" + "gitmek istediğim yer", ikisinin birleşimi.
        const ETG::FloatRect sweep{
            std::min(box.left, box.left + delta.x),
            std::min(box.top, box.top + delta.y),
            box.width + std::abs(delta.x),
            box.height + std::abs(delta.y)
        };

        CollectSolids(sweep, body->BlockingMask, body);
        if (Solids.empty()) return result;

        //> X ve Y hareketini aynı anda uygulayıp karakter duvarın içine girdikten sonra, yalnızca
        // > oluşan dikdörtgen çakışmasına bakarak hangi yüzeye çarptığını seçmek bazen belirsiz olabilir.
        //çözüm şu: Gerçek pozisyonu hemen değiştirme. Önce gitmek istediğin yerde geçici kutuyu dene, duvara giriyorsa izin verilen hareketi kısalt, sonra gerçek pozisyona uygula
        ETG::FloatRect resolved = box;

        if (delta.x != 0.f)
        {
            resolved.left = box.left + delta.x; //X hareketinin tamamını yapsaydım kutum nerede olurdu?

            //Cakisan her kati cismin soz hakki var ve en sikisi kazaniyor. Bunun yerine dongunun ICINDE kirpsaydik,
            //artik onumuzde olmayan bir duvar (cunku daha onceki biri bizi zaten disari cekmisti) cevabi belirleyebilirdi;
            //yani listede hangi cismin once geldigi onemli hale gelirdi
            float limit = resolved.left;
            for (const ETG::FloatRect& solid : Solids)
            {
                if (!resolved.intersects(solid)) continue;

                limit = delta.x > 0.f
                            ? std::min(limit, solid.left - resolved.width - ContactSkin) //sag kenarimiz onun sol yuzunun kil payi onunde
                            : std::max(limit, solid.left + solid.width + ContactSkin); //sol kenarimiz onun sag yuzunun kil payi onunde
            }

            if (limit != resolved.left)
            {
                resolved.left = limit;
                
                result.Velocity = Math::SlideAlongSurface(result.Velocity, {delta.x > 0.f ? -1.f : 1.f, 0.f});
                result.Blocked = true;
            }
        }

        if (delta.y != 0.f)
        {
            //Orijinal kutudan degil, az once cozulen X'ten devam: govdenin buradan yukari cikip cikamayacagi,
            //yanlamasina nerede kaldigina bagli - ve o, carptigi seyin icinden coktan cikarilmis durumda
            resolved.top = box.top + delta.y;

            float limit = resolved.top;
            for (const ETG::FloatRect& solid : Solids)
            {
                if (!resolved.intersects(solid)) continue;

                //Aşağı gidiyorsan → duvarın üstündeki güvenli top değerini seç.
                //Yukarı gidiyorsan → duvarın altındaki güvenli top değerini seç.
                limit = delta.y > 0.f
                            ? std::min(limit, solid.top - resolved.height - ContactSkin)
                            : std::max(limit, solid.top + solid.height + ContactSkin);
            }

            if (limit != resolved.top)
            {
                resolved.top = limit;

                //Ikinci cagri, ikinci birim normal - asla iki ekseni toplayip tek vektor yapmak degil. (-1,-1)
                //birim uzunlukta degildir; formul olandan iki kat fazlasini cikarir ve govdeyi koseden disari tukurur
                result.Velocity = Math::SlideAlongSurface(result.Velocity, {0.f, delta.y > 0.f ? -1.f : 1.f});
                result.Blocked = true;
            }
        }

        //Pozisyon olarak degil delta olarak geri veriliyor; boylece cagiran, zaten hareket ettirmekte oldugu seyin
        //uzerine eklemeye devam ediyor - sahibinin Position'i, yerel bir kopya, ya da bir force'un katkisi
        result.Delta = {resolved.left - box.left, resolved.top - box.top};
        return result;
    }

    //PHASE 5. This frame's contacts become the record the next frame compares against
    void CollisionSystem::Commit()
    {
        for (size_t i = 0; i < Active.size(); ++i)
        {
            CollisionComponent* component = Active[i];
            if (!component) continue;

            //Switched off partway through the dispatch above. SetCollisionEnabled has already announced the exits
            //and emptied the record; writing this frame's contacts over that would leave a disabled collider
            //holding a list of things it is "touching", and it would still be holding them on the frame somebody
            //switches it back on - at which point those contacts are old enough that no Enter would ever fire
            if (!component->IsCollisionEnabled())
            {
                component->StillColliding.Clear();
                continue;
            }

            //A swap and not a copy: the two lists hand buffers back and forth, so neither allocates again after
            //the first few frames
            component->PrevFrameCollisions.SwapWith(component->StillColliding);
        }
    }
}

2. Update/Draw listesi elle yazılmış, registry kullanılmıyor. GameManager.cpp:82-138'de her obje (Hero, Ak47, SawedOff, Magnum...) tek tek member olarak tutulup elle update/draw ediliyor. Halbuki SceneObjects registry'n zaten var — ama sadece ImGui hierarchy paneli için kullanılıyor. Bu modelde runtime'da obje spawn/destroy etmek mümkün değil, oysa Gungeon klonu demek oda oda düşman spawn'ı demek. Sahiplik merkezi bir vector<unique_ptr<GameObjectBase>>'e geçmeli, GameManager o listeyi dönmeli. Bu, projedeki en büyük mimari kısıt.

3. Input tamamen klavye+mouse'a gömülü. InputManager.cpp:25-31 doğrudan isKeyPressed(A/D/W/S) ve Mouse::isButtonPressed çağırıyor; kodda tek bir SDL_EVENT_FINGER_* yok. Bunu "MoveAxis / AimDirection / ShootPressed" gibi soyut aksiyonlara çevirip InputManager'ın altına klavye ve touch backend'leri koymadan mobilde oyun oynanamaz.

4. Frame pacing mobile uygun değil. RenderWindow.cpp:107-116 SDL_DelayNS ile elle 170 FPS limiti — mobilde bu pil yakar ve titrer. Doğrusu SDL_SetRenderVSync(renderer, 1); delay tabanlı limit sadece vsync kapalı masaüstü fallback'i olmalı.

Ciddi logic bugları

5. Focus dönüşünde delta-time patlaması. Globals::Update() sadece HasFocus iken çağrılıyor (GameManager.cpp:84-87), yani focus kaybında LastTickTime donuyor. Pencereye geri döndüğünde ilk FrameTick = odaksız geçen tüm süre. 30 saniye alt-tab yaptıysan dash timer'ları, bulletQueue.timeToFire, mermi hareketi — hepsi 30 saniyelik tek bir dt ile ilerler, her şey ışınlanır. Mobilde her background/foreground geçişinde yaşanacak. Fix basit: FrameTick'i clamp'le (örn. min(FrameTick, 0.1f)) veya focus geri gelince LastTickTime = now resetle.

6. GameState.h:55-57 — başlatılmamış pointerlar. ETG::RenderWindow* Window; ve Engine* Engine; default init edilmemiş; diğer üyeler = nullptr almış ama bu ikisi çöp değerle başlıyor. Setter çağrılmadan getter'a dokunan ilk kod UB. = nullptr ekle — iki karakterlik gerçek bug.

7. Registry ömür yönetimi yok. Factory.h'de kayıt raw pointer'la yapılıyor ama GameObjectBase destructor'ı kendini registry'den silmiyor. Projectiles kendini GunBase.cpp:158'de elle unregister ediyor, fakat Hero/gun/item gibi objeler yıkıldığında SceneObjs ve OrderedSceneObjs'ta dangling pointer kalır. Şu an her şey program ömrü boyunca yaşadığı için patlamıyor; ilk "sahne değiştir / düşman öldür" özelliğinde patlayacak. Ayrıca UnregisterGameObject'teki sceneObjs[name] (Factory.h:61) isim yoksa map'e nullptr ekler — find kullanılmalı. DestroyGameObject da unregister etmeden önce SetObjectNameToSelfClassName()'i tekrar çağırıyor; obje kendisi hâlâ kayıtlı olduğu için IncrementName yeni bir isim üretir ve yanlış anahtar silinir. Fonksiyon henüz kullanılmıyor ama kullandığın gün sessizce bozuk.

8. Kapanış sırası mayın tarlası. RenderWindow destructor'ı SDL_Quit() çağırıyor (RenderWindow.cpp:63), ama pencere Globals::Window static shared_ptr'ında da tutuluyor — yani yıkım static destruction'a sarkıyor. Üstüne DrawSinglePixelAtLoc'taki static ETG::Texture (Globals.cpp:60) renderer yok edildikten sonra yıkılıp SDL_DestroyTexture'ı ölü renderer'la çağırabilir. Event loop'taki Globals::Font.reset() (GameManager.cpp:154) tam da bu yüzden eklenmiş bir yara bandı. Doğrusu: deterministik bir Shutdown() fonksiyonu — tüm GPU kaynakları → ImGui → renderer/window → SDL_Quit, bu sırayla, main dönmeden önce.

9. İsim tabanlı kimlik + her spawn'da O(n) string taraması. IncrementName (GameObjectBase.cpp:122) her obje oluşturulduğunda tüm registry'de starts_with taraması yapıyor ve her mermi boost::typeindex pretty_name + string manipülasyonundan geçiyor. Gungeon klonu = saniyede onlarca mermi; bu, mobil CPU'da her atışta gereksiz allocation ve tarama demek. İsim ImGui paneli için kalsın ama kimlik monoton artan bir integer ID olmalı; mermiler için ayrıca object pool düşün.

Küçük ama değinmeye değer

- GameManager.cpp:105 clear({1,255,255,255}) — r=1, g=255? Muhtemelen niyet edilen renk bu değil.
- Globals.h:24 static int FPS = 170 — header'da static, her çeviri biriminde ayrı kopya oluşur; extern olmalı, yoksa bir gün bir TU'da değiştirip diğerinde eski değeri okursun.
- GameManager.h:56 static SDL_Event GameEvent — frame'deki son event'i tutuyor; bunu okuyan kod aynı frame'deki önceki event'leri kaçırır. Event'e ihtiyacı olan sisteme event'i o anda ilet, global'e yazma.
- iOS/Android lifecycle event'leri (SDL_EVENT_WILL_ENTER_BACKGROUND vs.) hâlâ ele alınmıyor — bunu önceki konuşmada konuşmuştuk, callback'lere geçiş + touch input refactor'üyle aynı pakette yapılmalı.

Önerdiğim sıra: önce 5 ve 6 (dakikalık fixler), sonra 1 (asset pipeline), sonra 2+7 birlikte (sahiplik modeli — ikisi aynı refactor), sonra 3 (input soyutlama). 8 ve 9'u sahiplik refactor'ü zaten büyük ölçüde çözer. İstersen bunlardan biriyle hemen başlayayım.
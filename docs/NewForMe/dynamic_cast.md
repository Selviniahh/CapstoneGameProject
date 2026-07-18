dynamic_cast<const BulletMan*>(obj)

şunu sorar:

> “Bu nesne BulletMan veya BulletMandan türemiş bir tür mü?”

Uygunsa pointer döner, değilse nullptr döner.

if (dynamic_cast<const BulletMan*>(obj))
{
// obj, BulletMan veya BulletMan'ın alt sınıfı
}

Eğer “tam olarak BulletMan olsun, alt sınıf olmasın” demek istiyorsan:

if (typeid(*obj) == typeid(BulletMan))
{
// Tam olarak BulletMan
}
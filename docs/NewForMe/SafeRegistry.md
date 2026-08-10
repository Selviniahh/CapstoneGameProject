# Ne bu 
Ne olduğu: içinde gezinirken değiştirilebilen bir pointer listesi.

Neden gerekiyor: collisionda çarpışma bulduğunda senin oyun kodun çalışıyor (Broadcast). O kod bir nesneyi yok edebilir. Nesne yok olunca component'i listeden çıkar. Ama biz o listenin ortasında geziniyoruz — ayağımızın altındaki halı çekiliyor.
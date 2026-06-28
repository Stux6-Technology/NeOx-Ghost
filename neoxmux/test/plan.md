# Neox Ghost Projesi - NeOxMUX Lokalize Sunucu ve RT IPS Sistemi

## NeOxMUX Lokalize Sunucu nedir ?
**NeOx-Ghost** Projesi kapsamında geliştirilmeye başlanan `NeoxMUX Lokalize Sunucu` yapısı `Ring 0` seviyesinde ağ anonimliği sağlayan birer sistem kütüphanesidir. Paketleri öncelikle lokalize sunucu da açtıklarını diğer sunuculara iletilerek farklı sunucularda 10ms ile ~20ms gib sürelerde beklettikten sorna geri çeker bu işlem paketlerin geliş noktalarını ve varış noktalarını saptırarak analiz edilmeyi imkansız hale getirir. **NeOxMUX Lokalize Sunucu** yapısı kendi içerisinde bir `Real Time Intrusion Prevention System` (RT IPS) sistemi kullanarak `I2P` ve `NeOxMUX` ile izi kapbettirilen paketleri derin analizden geçirerek `user-space` ya da `Kernel-Space`e iletir.

## `Real Time Intrusion Prevention System` nedir ?
`Real Time Intrusion Prevention System` **NeOxMUX** ile birlikte gelen paket tarama sistemidir. Güvenlik duvarının birinci katmanın yer alan RT IPS sistemi; `i2p` ve `NeOxMUX` sunucuları aracılığı ile izi kaybettirilen paketlerin geçip geçemeyeceğine karar veren bir derin araştırma aracıdır.

## NeOxMux Geliştirmesinin NeOx-Ghost'a etkisi nedir ?
**NeOxMUX** sisteminin geliştirilmesi NeOx Ghost'un ilk ve en önemli vaatlerinden olan **"Tamamen Anonimlik"** sözünü `Ring 0` seviyesine indirgemeyi sağlar. Bu durum da `Tor` ya da diğer `VPN` sağlayacıları ile yapılamayacak ve imkansız olan tamamen iz kaybettirme ve ağ tarama araçlarını (Wireshark vb.) tamamen atlatma vaadine tam uyar.

## Zamana göre değişen dosya yapısı:
**ilk dosya planması (28 Haziran 2026 / 11:10:21):**
```Plain Text
betik ve betik görevleri (28 Haziran 2026 / 11:10:21)

.neoxmux                --> Neox-Ghost ağ şifreleyicisi ve paket izlencisi (IPS)
├── crypto              --> Paket şifreleyici
│   ├── amk.c           --> Ana şifreleme kodu
│   ├── amk.h           --> şifreleme betiğine ait kütüphane kaynağı
│   └── amklib          --> Kernel iletişim protokollerinin bulunduğu klasör
│       ├── memory.s    --> RAM kontrolcüsü
│       └── passed.bf   --> İzoterik RAM kontrolcüsü
├── libkern.c           --> Kernel/lib kütüphanelerine erişim (baypass) sağlayan kaynak kod
├── memory.c            --> RAM Buffer vb. memory fatal'ları önleyen kaynak 
├── memory.h            --> RAM/MMU kaynak kütüphanesi
├── mux.c               --> lokal server başlatıcı ve i2p başlatıcısı
├── mux.h               --> lokal/i2p başlatıcı kütüphanesi
├── neoxmux.c           --> Ana kontrolcü
├── neoxmux.h           --> Ana kütüphane
├── __sub__.c           --> sub_hex brach kodu
├── sub.h               --> sub hex brach kütüphaneleri
├── sub.list            --> sub foksiyonları listesi
└── test
    ├── plan.html       --> `neoxmux` kodu bittikten evvel yazılacak olan son rapor
    └── plan.txt        --> `neoxmux` kütüphane/kernelib'sinin planlama dökümanı
```


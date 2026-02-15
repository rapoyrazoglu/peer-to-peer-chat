# PeerChat - Peer-to-Peer Chat Application Roadmap

> Torrent benzeri, tamamen dağıtık, merkezi sunucusu olmayan bir chat uygulaması.
> Dil: C++ (C++20)

---

## Faz 0: Proje Altyapısı

### 0.1 Build Sistemi
- [ ] CMake konfigürasyonu (minimum C++20)
- [ ] Proje dizin yapısı (`src/`, `include/`, `tests/`, `lib/`)
- [ ] Üçüncü parti bağımlılık yönetimi (CMake FetchContent veya vcpkg)
- [ ] CI/CD pipeline (GitHub Actions — build + test)

### 0.2 Temel Bağımlılıklar
- [ ] **asio** (standalone, Boost'suz) — async networking
- [ ] **libsodium** — şifreleme
- [ ] **spdlog** — loglama
- [ ] **nlohmann/json** — mesaj serializasyonu
- [ ] **googletest** — unit test

### 0.3 Geliştirme Ortamı
- [ ] `.clang-format` ve `.clang-tidy` konfigürasyonu
- [ ] `compile_commands.json` üretimi
- [ ] Basit bir `Makefile` wrapper (kolay build için)

---

## Faz 1: LAN Üzerinde İki Peer Arası Chat

> Hedef: Aynı ağdaki iki bilgisayar arasında düz metin mesajlaşma.

### 1.1 TCP Socket Katmanı
- [ ] Async TCP server (asio ile)
- [ ] Async TCP client
- [ ] Bağlantı yönetimi (connect, disconnect, reconnect)
- [ ] Basit mesaj framing protokolü (length-prefix)

### 1.2 Mesaj Protokolü v1
- [ ] Mesaj yapısı tanımı (JSON):
  ```json
  {
    "type": "text|handshake|ack|ping|pong",
    "sender_id": "unique-peer-id",
    "timestamp": 1234567890,
    "payload": "mesaj içeriği"
  }
  ```
- [ ] Handshake mekanizması (peer kimlik değişimi)
- [ ] Mesaj ACK sistemi (iletildi onayı)
- [ ] Ping/Pong heartbeat

### 1.3 CLI Arayüzü v1
- [ ] Terminal tabanlı basit chat arayüzü
- [ ] Komut sistemi (`/connect <ip:port>`, `/quit`, `/status`)
- [ ] Gelen/giden mesaj gösterimi (zaman damgalı)

### 1.4 Peer Kimliği
- [ ] Rastgele unique peer ID üretimi (ilk çalıştırmada)
- [ ] Peer ID'yi diske kaydetme (`~/.peerchat/identity`)
- [ ] Kullanıcı adı (nickname) desteği

**Milestone: İki terminal açıp IP ile birbirine bağlanarak mesajlaşabilme.**

---

## Faz 2: Şifreleme

> Hedef: Uçtan uca şifrelenmiş iletişim. Kimse araya girip okuyamasın.

### 2.1 Anahtar Yönetimi
- [ ] X25519 anahtar çifti üretimi (public/private key)
- [ ] Anahtarları diske güvenli kaydetme
- [ ] Public key'i peer kimliğinin parçası yapma

### 2.2 Anahtar Değişimi
- [ ] X25519 Diffie-Hellman anahtar değişimi (handshake sırasında)
- [ ] Shared secret türetme (HKDF)
- [ ] Perfect Forward Secrecy için session key rotasyonu

### 2.3 Mesaj Şifreleme
- [ ] XChaCha20-Poly1305 ile mesaj şifreleme/deşifreleme
- [ ] Nonce yönetimi (tekrarlanmayan)
- [ ] Şifreli mesaj framing

### 2.4 Kimlik Doğrulama
- [ ] Peer public key fingerprint gösterimi
- [ ] İlk bağlantıda "Trust on First Use" (TOFU) modeli
- [ ] Güvenilen peer listesi (`~/.peerchat/trusted_peers`)

**Milestone: Wireshark ile bakıldığında mesajlar okunamaz durumda.**

---

## Faz 3: Çoklu Peer & Mesaj Yönlendirme

> Hedef: İkiden fazla peer'ın aynı anda bağlı olması ve mesaj iletimi.

### 3.1 Bağlantı Havuzu
- [ ] Birden fazla eşzamanlı peer bağlantısı
- [ ] Bağlantı limiti ve yönetimi
- [ ] Peer listesi ve durumları (online/offline/connecting)

### 3.2 Mesaj Yönlendirme (Relay)
- [ ] A, B'ye bağlı; B, C'ye bağlı → A, C'ye mesaj gönderebilir (B üzerinden)
- [ ] TTL (Time to Live) ile sonsuz döngü önleme
- [ ] Mesaj ID ile duplicate tespiti
- [ ] Flooding yerine akıllı routing (gossip protocol temeli)

### 3.3 Grup Chat
- [ ] Grup oluşturma ve gruba katılma
- [ ] Grup mesajı yayınlama (broadcast)
- [ ] Grup üye listesi senkronizasyonu
- [ ] Grup bazlı şifreleme (grup shared key)

### 3.4 CLI Arayüzü v2
- [ ] Aktif peer listesi görüntüleme
- [ ] Peer seçerek özel mesaj (`/msg <peer> <mesaj>`)
- [ ] Grup komutları (`/group create`, `/group join`, `/group list`)
- [ ] ncurses tabanlı gelişmiş TUI (split-screen: mesajlar + input)

**Milestone: 3+ kişi LAN üzerinde aynı anda grup sohbeti yapabilir.**

---

## Faz 4: Peer Discovery (Birbirini Bulma)

> Hedef: IP adresini elle yazmadan aynı ağdaki peer'ları otomatik keşfetme.

### 4.1 LAN Discovery
- [ ] UDP broadcast/multicast ile LAN'daki peer'ları bulma
- [ ] mDNS/DNS-SD ile servis duyurusu (opsiyonel)
- [ ] Otomatik peer listesi oluşturma

### 4.2 Bootstrap Nodes
- [ ] Bilinen bootstrap node listesi (hardcoded + config dosyası)
- [ ] Bootstrap node'a bağlanıp aktif peer listesi alma
- [ ] Kendini bootstrap node olarak duyurabilme

### 4.3 DHT (Distributed Hash Table) — Kademlia
- [ ] Kademlia protokolü implementasyonu
  - [ ] Node ID (256-bit)
  - [ ] XOR distance metric
  - [ ] k-bucket routing table
  - [ ] FIND_NODE, FIND_VALUE, STORE, PING RPC'leri
- [ ] Peer bilgilerini DHT'ye kaydetme
- [ ] DHT üzerinden peer arama
- [ ] Routing table persistence (diske kaydetme)

### 4.4 Peer Exchange (PEX)
- [ ] Bağlı olduğun peer'lardan onların bildiği peer'ları öğrenme
- [ ] Periyodik peer listesi paylaşımı
- [ ] Kötü niyetli peer filtresi (basit reputation)

**Milestone: Sadece bir bootstrap node bilgisiyle ağa katılıp peer bulabilme.**

---

## Faz 5: NAT Traversal (İnternet Üzerinde Çalışma)

> Hedef: Farklı ağlardaki (evlerindeki) kullanıcıların birbirine bağlanması.

### 5.1 STUN
- [ ] STUN client implementasyonu (RFC 5389)
- [ ] Public IP ve port tespiti
- [ ] NAT tipi tespiti (Full Cone, Restricted, Symmetric)

### 5.2 UDP Hole Punching
- [ ] UDP socket'ler ile hole punching
- [ ] Koordinasyon mekanizması (bir aracı peer veya bootstrap üzerinden)
- [ ] Farklı NAT tipleri için farklı stratejiler
- [ ] Başarısız olursa fallback planı

### 5.3 TURN (Relay Fallback)
- [ ] TURN client implementasyonu (RFC 5766)
- [ ] Hole punching başarısız olursa relay üzerinden bağlantı
- [ ] TURN sunucu maliyeti farkındalığı (topluluk destekli?)

### 5.4 UPnP / NAT-PMP
- [ ] UPnP ile otomatik port forwarding talebi
- [ ] NAT-PMP desteği
- [ ] Port mapping durumu gösterimi

### 5.5 Bağlantı Katmanı Soyutlama
- [ ] Transport interface: Direct TCP, UDP Hole Punch, TURN Relay
- [ ] Otomatik en iyi yöntem seçimi
- [ ] Bağlantı kalitesi metrikleri (latency, packet loss)

**Milestone: Farklı evlerdeki iki bilgisayar internet üzerinden chat edebilir.**

---

## Faz 6: Güvenilirlik & Offline Destek

> Hedef: Mesaj kaybı olmaması ve offline kullanıcılara mesaj iletimi.

### 6.1 Mesaj Depolama
- [ ] SQLite ile yerel mesaj veritabanı
- [ ] Mesaj geçmişi sorgulama
- [ ] Chat log export (düz metin)

### 6.2 Store-and-Forward
- [ ] Offline peer'a gönderilen mesajları geçici olarak tutma
- [ ] Peer online olduğunda birikmiş mesajları iletme
- [ ] Birden fazla aracı peer'da mesaj kopyası tutma (redundancy)
- [ ] Mesaj TTL (ne kadar süre tutulacak)

### 6.3 Mesaj Senkronizasyonu
- [ ] Vector clock veya Lamport timestamp ile mesaj sıralaması
- [ ] Eksik mesaj tespiti ve yeniden isteme
- [ ] Çakışma çözümü (conflict resolution)

### 6.4 Bağlantı Dayanıklılığı
- [ ] Otomatik yeniden bağlanma (exponential backoff)
- [ ] Bağlantı durumu izleme
- [ ] Graceful disconnect (çıkış bildirimi)

**Milestone: Peer offline iken gönderilen mesajlar, online olunca ulaşır.**

---

## Faz 7: Dosya Transferi

> Hedef: Peer'lar arası dosya gönderimi (torrent mantığına benzer).

### 7.1 Basit Dosya Transferi
- [ ] Küçük dosyaları doğrudan gönderme
- [ ] İlerleme gösterimi
- [ ] Dosya bütünlüğü kontrolü (SHA-256 hash)

### 7.2 Parçalı Transfer (Torrent-style)
- [ ] Büyük dosyaları chunk'lara bölme
- [ ] Birden fazla peer'dan paralel indirme
- [ ] Chunk doğrulama (Merkle tree)
- [ ] Eksik chunk'ları farklı peer'dan isteme

### 7.3 Dosya Paylaşımı
- [ ] Paylaşılan dosya listesi (shared folder)
- [ ] Dosya arama (DHT üzerinden)
- [ ] Magnet link benzeri paylaşım linki

**Milestone: Büyük bir dosyayı birden fazla peer'dan paralel indirebilme.**

---

## Faz 8: Gelişmiş Özellikler

### 8.1 GUI (Opsiyonel)
- [ ] ImGui veya Qt ile masaüstü arayüz
- [ ] Konuşma baloncukları, bildirimler
- [ ] Sistem tepsisi (tray) ikonu
- [ ] Tema desteği (dark/light)

### 8.2 Ses/Video (İleri Seviye)
- [ ] Opus codec ile ses iletimi
- [ ] WebRTC benzeri P2P ses kanalı
- [ ] Push-to-talk ve VOX modu

### 8.3 Plugin Sistemi
- [ ] Basit plugin API'si (shared library / .so / .dll)
- [ ] Bot desteği (otomatik yanıtlayıcı)
- [ ] Custom komutlar

### 8.4 Tor / I2P Entegrasyonu
- [ ] Tor hidden service olarak çalışabilme
- [ ] I2P üzerinden bağlantı desteği
- [ ] Tam anonim mod

---

## Teknik Mimari (Özet)

```
┌─────────────────────────────────────────────┐
│                 Application                  │
│         (CLI / TUI / GUI interface)          │
├─────────────────────────────────────────────┤
│              Chat Protocol                   │
│    (messages, groups, file transfer)         │
├─────────────────────────────────────────────┤
│            Encryption Layer                  │
│     (X25519 + XChaCha20-Poly1305)           │
├─────────────────────────────────────────────┤
│           Routing & Discovery                │
│      (DHT / PEX / Bootstrap / mDNS)         │
├─────────────────────────────────────────────┤
│           Connection Manager                 │
│  (TCP / UDP Hole Punch / TURN / UPnP)       │
├─────────────────────────────────────────────┤
│              Network Layer                   │
│           (asio / raw sockets)               │
└─────────────────────────────────────────────┘
```

---

## Tahmini Faz Sıralaması

| Faz | İçerik | Önkoşul |
|-----|--------|---------|
| 0 | Proje altyapısı | — |
| 1 | LAN TCP chat | Faz 0 |
| 2 | Şifreleme | Faz 1 |
| 3 | Çoklu peer & grup | Faz 1 |
| 4 | Peer discovery (DHT) | Faz 3 |
| 5 | NAT traversal | Faz 4 |
| 6 | Offline & güvenilirlik | Faz 3 |
| 7 | Dosya transferi | Faz 5 |
| 8 | Gelişmiş özellikler | Faz 5+ |

> Faz 2 ve 3 paralel geliştirilebilir.
> Faz 6 ve 7 de paralel geliştirilebilir.

---

## Lisans

MIT License — açık kaynak, herkes katkıda bulunabilir.

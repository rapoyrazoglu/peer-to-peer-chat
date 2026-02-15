# PeerChat

Torrent-style, fully decentralized, serverless peer-to-peer chat application written in C++20.

No central server. No data collection. No censorship. Just peers talking to peers.

## Features (Planned)

- Direct peer-to-peer messaging over TCP/UDP
- End-to-end encryption (X25519 + XChaCha20-Poly1305)
- NAT traversal (STUN, UDP hole punching, UPnP)
- Distributed peer discovery (Kademlia DHT)
- Group chat support
- Offline message delivery (store-and-forward)
- Torrent-style file transfer (chunked, parallel)
- Terminal UI (ncurses)

## Building

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

## Usage

```bash
./build/peerchat
```

## Roadmap

See [ROADMAP.md](ROADMAP.md) for the full development plan.

## License

MIT

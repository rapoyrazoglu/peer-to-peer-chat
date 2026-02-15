# PeerChat

Torrent-style, fully decentralized, serverless peer-to-peer chat application written in C++20.

No central server. No data collection. No censorship. Just peers talking to peers.

## Features

- Direct peer-to-peer messaging over TCP
- JSON wire protocol with length-prefixed framing
- Handshake, ACK, and heartbeat (ping/pong)
- Persistent peer identity (UUID v4)
- CLI with `/connect`, `/disconnect`, `/status`, `/quit`

### Planned

- End-to-end encryption (X25519 + XChaCha20-Poly1305)
- NAT traversal (STUN, UDP hole punching, UPnP)
- Distributed peer discovery (Kademlia DHT)
- Group chat support
- Offline message delivery (store-and-forward)
- Torrent-style file transfer (chunked, parallel)
- Terminal UI (ncurses)

## Installation

### macOS (Homebrew)

```bash
brew install --formula packaging/homebrew/peerchat.rb
```

Or tap from the repository:

```bash
brew tap rapoyrazoglu/peerchat https://github.com/rapoyrazoglu/peer-to-peer-chat
brew install peerchat
```

### Arch Linux (AUR)

```bash
# Using an AUR helper (e.g. yay)
yay -S peerchat

# Or manually
git clone https://aur.archlinux.org/peerchat.git
cd peerchat
makepkg -si
```

### Windows

Download `peerchat-windows-x86_64.zip` from the [latest release](https://github.com/rapoyrazoglu/peer-to-peer-chat/releases/latest), extract, and run `peerchat.exe`.

### Building from Source

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
```

**Dependencies:** CMake 3.20+, C++20 compiler, libsodium (optional)

## Usage

```bash
# Terminal 1
./peerchat --port 9000 --nick alice

# Terminal 2
./peerchat --port 9001 --nick bob
```

In bob's terminal:

```
/connect 127.0.0.1:9000
```

Then just type messages. Commands:

| Command | Description |
|---------|-------------|
| `/connect <host>:<port>` | Connect to a peer |
| `/disconnect` | Disconnect from peer |
| `/status` | Show connection info |
| `/quit` | Exit PeerChat |

## Roadmap

See [ROADMAP.md](ROADMAP.md) for the full development plan.

## License

MIT

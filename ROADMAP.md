# PeerChat - Peer-to-Peer Chat Application Roadmap

> Torrent-style, fully decentralized, serverless chat application.
> Language: C++ (C++20)

---

## Phase 0: Project Infrastructure

### 0.1 Build System
- [x] CMake configuration (minimum C++20)
- [x] Project directory structure (`src/`, `include/`, `tests/`, `lib/`)
- [x] Third-party dependency management (CMake FetchContent)
- [x] CI/CD pipeline (GitHub Actions — build + test)

### 0.2 Core Dependencies
- [x] **asio** (standalone, no Boost) — async networking
- [x] **libsodium** — encryption
- [x] **spdlog** — logging
- [x] **nlohmann/json** — message serialization
- [x] **googletest** — unit testing

### 0.3 Development Environment
- [x] `.clang-format` and `.clang-tidy` configuration
- [x] `compile_commands.json` generation
- [ ] Simple `Makefile` wrapper (for easy builds)

---

## Phase 1: LAN Two-Peer TCP Chat

> Goal: Plain text messaging between two computers on the same network.

### 1.1 TCP Socket Layer
- [x] Async TCP server (with asio)
- [x] Async TCP client
- [x] Connection management (connect, disconnect, reconnect)
- [x] Simple message framing protocol (length-prefix)

### 1.2 Message Protocol v1
- [x] Message structure definition (JSON):
  ```json
  {
    "type": "text|handshake|ack|ping|pong",
    "sender_id": "unique-peer-id",
    "timestamp": 1234567890,
    "payload": "message content"
  }
  ```
- [x] Handshake mechanism (peer identity exchange)
- [x] Message ACK system (delivery confirmation)
- [x] Ping/Pong heartbeat

### 1.3 CLI Interface v1
- [x] Simple terminal-based chat interface
- [x] Command system (`/connect <ip:port>`, `/quit`, `/status`)
- [x] Incoming/outgoing message display (timestamped)

### 1.4 Peer Identity
- [x] Random unique peer ID generation (on first run)
- [x] Save peer ID to disk (`~/.peerchat/identity`)
- [x] Username (nickname) support

**Milestone: Open two terminals, connect via IP, and exchange messages.**

---

## Phase 2: Encryption

> Goal: End-to-end encrypted communication. No one can intercept and read messages.

### 2.1 Key Management
- [ ] X25519 key pair generation (public/private key)
- [ ] Securely save keys to disk
- [ ] Make public key part of peer identity

### 2.2 Key Exchange
- [ ] X25519 Diffie-Hellman key exchange (during handshake)
- [ ] Shared secret derivation (HKDF)
- [ ] Session key rotation for Perfect Forward Secrecy

### 2.3 Message Encryption
- [ ] Message encryption/decryption with XChaCha20-Poly1305
- [ ] Nonce management (non-repeating)
- [ ] Encrypted message framing

### 2.4 Authentication
- [ ] Peer public key fingerprint display
- [ ] Trust on First Use (TOFU) model on first connection
- [ ] Trusted peer list (`~/.peerchat/trusted_peers`)

**Milestone: Messages are unreadable when inspected with Wireshark.**

---

## Phase 3: Multi-Peer & Message Routing

> Goal: More than two peers connected simultaneously with message relay.

### 3.1 Connection Pool
- [ ] Multiple simultaneous peer connections
- [ ] Connection limits and management
- [ ] Peer list and states (online/offline/connecting)

### 3.2 Message Routing (Relay)
- [ ] A connected to B; B connected to C → A can message C (through B)
- [ ] TTL (Time to Live) to prevent infinite loops
- [ ] Duplicate detection via message ID
- [ ] Smart routing instead of flooding (gossip protocol basis)

### 3.3 Group Chat
- [ ] Group creation and joining
- [ ] Group message broadcast
- [ ] Group member list synchronization
- [ ] Group-level encryption (group shared key)

### 3.4 CLI Interface v2
- [ ] Active peer list display
- [ ] Direct message by selecting peer (`/msg <peer> <message>`)
- [ ] Group commands (`/group create`, `/group join`, `/group list`)
- [ ] ncurses-based advanced TUI (split-screen: messages + input)

**Milestone: 3+ people can have a group conversation on a LAN.**

---

## Phase 4: Peer Discovery

> Goal: Automatically discover peers on the same network without manually entering IPs.

### 4.1 LAN Discovery
- [ ] Find LAN peers via UDP broadcast/multicast
- [ ] Service announcement via mDNS/DNS-SD (optional)
- [ ] Automatic peer list generation

### 4.2 Bootstrap Nodes
- [ ] Known bootstrap node list (hardcoded + config file)
- [ ] Connect to bootstrap node and get active peer list
- [ ] Ability to announce self as a bootstrap node

### 4.3 DHT (Distributed Hash Table) — Kademlia
- [ ] Kademlia protocol implementation
  - [ ] Node ID (256-bit)
  - [ ] XOR distance metric
  - [ ] k-bucket routing table
  - [ ] FIND_NODE, FIND_VALUE, STORE, PING RPCs
- [ ] Store peer information in DHT
- [ ] Peer lookup via DHT
- [ ] Routing table persistence (save to disk)

### 4.4 Peer Exchange (PEX)
- [ ] Learn peers known by connected peers
- [ ] Periodic peer list sharing
- [ ] Malicious peer filter (basic reputation)

**Milestone: Join the network and discover peers with just bootstrap node info.**

---

## Phase 5: NAT Traversal (Working Over the Internet)

> Goal: Users on different networks (at home) can connect to each other.

### 5.1 STUN
- [ ] STUN client implementation (RFC 5389)
- [ ] Public IP and port detection
- [ ] NAT type detection (Full Cone, Restricted, Symmetric)

### 5.2 UDP Hole Punching
- [ ] Hole punching with UDP sockets
- [ ] Coordination mechanism (via a relay peer or bootstrap)
- [ ] Different strategies for different NAT types
- [ ] Fallback plan if unsuccessful

### 5.3 TURN (Relay Fallback)
- [ ] TURN client implementation (RFC 5766)
- [ ] Relay connection when hole punching fails
- [ ] TURN server cost awareness (community-supported?)

### 5.4 UPnP / NAT-PMP
- [ ] Automatic port forwarding request via UPnP
- [ ] NAT-PMP support
- [ ] Port mapping status display

### 5.5 Connection Layer Abstraction
- [ ] Transport interface: Direct TCP, UDP Hole Punch, TURN Relay
- [ ] Automatic best method selection
- [ ] Connection quality metrics (latency, packet loss)

**Milestone: Two computers in different homes can chat over the internet.**

---

## Phase 6: Reliability & Offline Support

> Goal: No message loss and message delivery to offline users.

### 6.1 Message Storage
- [ ] Local message database with SQLite
- [ ] Message history querying
- [ ] Chat log export (plain text)

### 6.2 Store-and-Forward
- [ ] Temporarily hold messages sent to offline peers
- [ ] Deliver accumulated messages when peer comes online
- [ ] Store message copies on multiple relay peers (redundancy)
- [ ] Message TTL (how long to retain)

### 6.3 Message Synchronization
- [ ] Message ordering with vector clocks or Lamport timestamps
- [ ] Missing message detection and re-request
- [ ] Conflict resolution

### 6.4 Connection Resilience
- [ ] Automatic reconnection (exponential backoff)
- [ ] Connection state monitoring
- [ ] Graceful disconnect (exit notification)

**Milestone: Messages sent while peer is offline are delivered when they come online.**

---

## Phase 7: File Transfer

> Goal: File transfer between peers (similar to torrent logic).

### 7.1 Simple File Transfer
- [ ] Direct small file sending
- [ ] Progress display
- [ ] File integrity check (SHA-256 hash)

### 7.2 Chunked Transfer (Torrent-style)
- [ ] Split large files into chunks
- [ ] Parallel download from multiple peers
- [ ] Chunk verification (Merkle tree)
- [ ] Request missing chunks from different peers

### 7.3 File Sharing
- [ ] Shared file list (shared folder)
- [ ] File search (via DHT)
- [ ] Magnet link-style sharing link

**Milestone: Download a large file in parallel from multiple peers.**

---

## Phase 8: Advanced Features

### 8.1 GUI (Optional)
- [ ] Desktop interface with ImGui or Qt
- [ ] Chat bubbles, notifications
- [ ] System tray icon
- [ ] Theme support (dark/light)

### 8.2 Voice/Video (Advanced)
- [ ] Audio transmission with Opus codec
- [ ] WebRTC-like P2P voice channel
- [ ] Push-to-talk and VOX mode

### 8.3 Plugin System
- [ ] Simple plugin API (shared library / .so / .dll)
- [ ] Bot support (auto-responder)
- [ ] Custom commands

### 8.4 Tor / I2P Integration
- [ ] Run as Tor hidden service
- [ ] Connection support over I2P
- [ ] Full anonymous mode

---

## Technical Architecture (Summary)

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

## Estimated Phase Order

| Phase | Content | Prerequisite |
|-------|---------|--------------|
| 0 | Project infrastructure | — |
| 1 | LAN TCP chat | Phase 0 |
| 2 | Encryption | Phase 1 |
| 3 | Multi-peer & groups | Phase 1 |
| 4 | Peer discovery (DHT) | Phase 3 |
| 5 | NAT traversal | Phase 4 |
| 6 | Offline & reliability | Phase 3 |
| 7 | File transfer | Phase 5 |
| 8 | Advanced features | Phase 5+ |

> Phases 2 and 3 can be developed in parallel.
> Phases 6 and 7 can also be developed in parallel.

---

## License

MIT License — open source, everyone can contribute.

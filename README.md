# Spacewar

A short **modern C++** gameplay sample inspired by the classic **Spacewar!**.  
Demonstrates clean system decomposition, data-driven gameplay, and engineering tradeoffs relevant to real-world game development.

The project showcases:
- Decoupled input via an **event bus**
- Data-driven **ECS-style components**
- Gameplay **systems** (movement, collision, rendering)
- Safety, performance, and determinism considerations suitable for larger-scale or multiplayer-like simulations

---

## Goals

Demonstrate solid modern C++ engineering practices:

- Clear separation of **data vs. behaviour** (component storage + systems)
- Decoupled input delivery using **publish/subscribe**
- Determinism and **testability** (fixed tick, event replay)
- Scalable, cache-friendly component storage
- Realistic performance and threading constraints

---

## Architecture Overview

### High-Level Layers

- **Input layer**
  - SFML input is translated into `KeyEvent` / `MouseEvent`
  - Events are published via `Game::handleInput()` to `GlobalEventBus()`

- **Event Bus**
  - Systems and gameplay logic subscribe to relevant events
  - Example:
    - `PlayerSystem` consumes `KeyEvent`
    - `Game` consumes `MouseEvent` for shooting

- **ECS-Style Data Model**
  - `AsteroidComponentManager` stores **pure data** (`AsteroidComponent`)
  - `Asteroid` entity is a thin wrapper owning a component ID
  - Systems operate on component data, not entities

- **Systems**
  - `MovementSystem`: updates positions from component data
  - `CollisionSystem`: reads component radius and transforms
  - `RenderSystem`: consumes component shape copies and renders via SFML on the main thread

---

## Key Design Choices & Rationale

### Event Bus (Decoupling)

**Benefits**
- Loose coupling between input, UI, and gameplay
- Easier unit testing
- Explicit and traceable input flow

**Tradeoffs**
- Avoid overuse on hot paths (e.g. per-frame polling)
- Event ordering must be well-defined

---

### Data-Driven Components (ECS Approach)

- Components store **data only**:
  - position, radius, speed, direction, render primitive
- Systems operate on component snapshots
- Improves batch processing and reduces coupling

**Current implementation**
- `unordered_map` with locking (simple, safe)

**Production alternative**
- Dense `std::vector` + free-list (swap-remove)
- Better cache locality and iteration speed

---

### Threading & Safety

- Component storage protected via `std::shared_mutex`
- Render thread receives **copies only** (`getShapeCopy`)
- No raw internal data is shared across threads

---

### Determinism & Testability

- Input is deterministic:
  - capture `KeyEvent` / `MouseEvent` sequences with timestamps
  - replay events on a fixed tick
- Simulation uses a **fixed timestep**, rendering is decoupled

---

### Performance & Scaling

- Replace `unordered_map` with contiguous storage for hot paths
- Consider **SoA (Structure of Arrays)** for frequently-iterated data
- Avoid per-frame allocations:
  - `ObjectPool<T>`
  - arenas for components
- Use spatial partitioning to reduce `O(N*M)` collision checks

---

### Modern C++ Practices

- `constexpr` where applicable
- RAII for resource management
- `noexcept` on small, non-throwing functions
- Value semantics for small types
- Move semantics where appropriate
- `std::shared_mutex`, `std::atomic` for thread safety
- No raw `new` / `delete`

**Design note**
- Constructor injection (`EventBus&`) preferred for testability  
- The sample uses `GlobalEventBus()` for simplicity

---

## Summary

This project is intentionally small, but designed with **real production constraints** in mind:
- scalability
- determinism
- threading
- clean C++ architecture

It serves as a focused gameplay/UI engineering sample rather than a full game.

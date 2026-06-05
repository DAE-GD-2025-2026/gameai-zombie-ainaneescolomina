# Autonomous Survivor AI — Unreal Engine Framework

The objective of this project is to maximize the lifespan of an autonomous survival agent within a volatile threat ecosystem. By separating cognitive decision-making from locomotive physics, the system evaluates internal vital stats, manages structural navigation memory, and applies fluid steering forces to independently balance resource scavenging, tactical combat, and adaptive hazard avoidance.

---

## 🚀 Architectural Overview

### 1. Perception & Senses
* **UE AI Perception:** Actively monitors the environment for targets using simulated sight and hearing configurations.
* **Timestamped Tracking Memory:** Implements a localized memory map to log threat coordinates, preventing immediate state loss and ensuring target persistence after line-of-sight breaks.

### 2. Action Selection & Inventory Control
* **Smart Inventory Sorting:** A background service (`URefreshStatsServiceNeesAina`) constantly watches the agents health and stamina values while scanning inventory slots to classify objects into Food, Medkits, or Weapons.
* **Auto-Consuming Items:** The AI directly tracks its inventory slot numbers. If its stats drop dangerously low, it automatically uses a medkit or eats food and prioritizing nearby items using a `KnownItems` lookup list.

---

## 🧠 Behavior Tree Logic Hierarchy

The Behavior Tree uses left-to-right priority branching with **Observer Aborts** (`aborts lower priority`) to react to Blackboard configurations.

* **Branch 1: Purge Evasion:** The absolute highest priority. Instantly triggers if the agent is caught inside a purge zone, forcing the agent to escape the area immediately while sprinting.
* **Branch 2: Combat & Threat Handling (`Zombie is Set`):** Activated when zombies are detected. Armed agents halt, rotate, and shoot targets, while unarmed agents automatically trigger sprint logic to execute a tactical retreat.
* **Branch 3: Low Needs:** Constantly monitors vital stats. If health or stamina drops below a thresholds, it overrides lower tasks to consume food or medkits from the inventory.
* **Branch 4: Item Looting (`TargetItem is Set`):** Executes `Seek Item Steering` to drive towards an item's coordinates, and runs `Pick Up Target Item`, and deletes garbage to free up world space.
* **Branch 5: House Looting (`TargetHouse is Set`):** Triggeres `Loot House Steering` to navigate to a building's known closest entrance point or the center position, and triggers `Visit House` to clear blackboard keys upon entry.
* **Branch 6: Baseline Wandering:** The lowest priority fallback task. Does a standard `Wander Steering` sequence behavior to keep the agent in motion.

---

## 🛠️ Custom Blended Steering Engine

* **Dynamic Weight Mixing:** The blending component (`BlendedSteeringNeesAina`) compiles multiple active steering behaviors simultaneously, multiplying each force (`Seek`, `Flee`, `Evade`, `Wander`) by a specific weight percentage to output a final velocity vector.
* **Remembering Houses:** It flags houses it has already looted so it seeks new houses, and it remembers door coordinates (`DiscoveredEntrances`) so the steering system can head straight for the closest known entryway.
* **Anti-Stuck Logic:** If the movement system detects that the agent's physical position hasn't changed for more than `1.0` second, it realizes it's stuck on geometry and automatically forces a $90^\circ$ detour angle to try and walk around the obstacle.
* **Panic Angle Modifier:** If the agent stays trapped for three times longer (`3.0` seconds), the recovery logic automatically adds to the detour angle an extra $45^\circ$, forcing a extreme turn to break the agent out of tight corners.

---

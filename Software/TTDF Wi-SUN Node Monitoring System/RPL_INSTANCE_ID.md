# Understanding Instance ID and RPL in Wi-SUN Networks

A comprehensive guide explaining `instance_id`, `dodag_rank`, RPL routing strategies, and MRHOF based on real network observations.

---

## What is `instance_id`?

### Technical Definition

In the context of Wi-SUN and RPL (Routing Protocol for Low-Power and Lossy Networks), the `instance_id` is a unique identifier set by the DODAG (Destination-Oriented Directed Acyclic Graph) root. It indicates which specific RPL Instance the DODAG belongs to.

In an RPL network, multiple DODAGs can exist, and the `instance_id` allows the protocol to distinguish between different routing topologies or optimization objectives within the same network. When you see an `instance_id` of 0 alongside a `dodag_rank` of 384, it means the node is participating in the RPL instance identified as "0" and has calculated its logical distance (rank) from the root within that specific topology.

If you are monitoring network statistics, you might also encounter `rpl_unknown_instance`, which counts packets discarded because they reference an `instance_id` that the node does not recognize.

---

## Simple Explanation

### What is `instance_id`? (in simple words)

Think of RPL routing like having **multiple road maps** in the same city.

- **Each road map** = one RPL Instance
- **instance_id** = the map number

So when you see:
```json
"instance_id": 0
```

👉 It means the node is using **routing map number 0**

### Why do we need `instance_id`?

A Wi-SUN network can run **more than one routing strategy** at the same time, for example:

- One for **normal traffic**
- One for **backup / emergency**
- One optimized for **low power**
- One optimized for **low latency**

Each strategy gets its own `instance_id` so nodes don't mix them up.

### What does this mean with `dodag_rank`?

```json
"instance_id": 0,
"dodag_rank": 384
```

This means:

- The node belongs to **routing instance 0**
- Its **distance from the root** in that routing map is **384**
  - Lower rank = closer to the border router
  - Higher rank = further away

**Example:**

- Root → rank ~128
- One hop away → ~256
- Two hops away → ~384

So your node is likely **2–3 hops from the root**.

### Simple Analogy

| RPL term | Real-life meaning |
|----------|-------------------|
| Root | City center |
| DODAG | Road network |
| instance_id | Map number |
| dodag_rank | How far you are from city center |

### About `rpl_unknown_instance`

If you see:
```
rpl_unknown_instance
```

It simply means:

**"I received a packet for a map number I don't know, so I dropped it."**

### Final One-Line Explanation

👉 **`instance_id` tells the node which routing map it is using.**  
👉 **`dodag_rank` tells how far the node is from the root in that map.**

---

## Important Clarification: Paths vs Instances

### ❌ Common Misunderstanding

"So basically there are multiple ways of reaching the root node, so multiple ways have unique instance IDs"

### ✅ Correct Understanding

**❌ Not multiple ways = multiple instance_id**

**✅ Multiple routing strategies = multiple instance_id**

Let me explain the difference:

### 1️⃣ Multiple Paths to the Root

Yes, in Wi-SUN/RPL there are **multiple ways (paths)** to reach the root.

👉 But **all those paths belong to the SAME instance_id**.

- Instance ID stays the same
- RPL automatically chooses the best parent/path
- Backup paths exist inside the same instance

### 2️⃣ What Actually Creates Multiple `instance_id`s?

A new `instance_id` is created **only when the root decides to run a different routing objective**.

Examples:

- One instance optimized for **lowest latency**
- Another instance optimized for **lowest power**
- Another instance for **reliable traffic**

Each such routing "mode" gets a different `instance_id`.

### Simple Analogy (Very Clear)

**Same `instance_id`:**
- 🗺️ One Google Map
- 🚗 Multiple roads to reach home
- ➡️ Map stays the same, roads differ

**Different `instance_id`:**
- 🗺️ Google Maps vs Waze vs Offline Map
- ➡️ Same destination, different routing logic

### So in Your Network

```json
"instance_id": 0
```

Means:

- Only **one routing strategy** is running
- Multiple paths may exist, but all are inside instance 0
- This is **standard for Wi-SUN** — almost all deployments use only instance 0

### One-Line Correction

❌ "Multiple ways have unique instance id"  
✅ **"Multiple routing strategies have unique instance id; multiple paths share the same instance id."**

---

## Understanding Routing Strategies

### What is a Routing Strategy?

A **routing strategy** means the **rule used to decide which path is "best"** to reach the root (Border Router).

Examples of different strategies:

- Choose path with **lowest delay**
- Choose path with **strongest signal**
- Choose path that **saves battery**
- Choose path that is **most reliable**

Each strategy answers the question:

**"How should I choose my parent?"**

### What Does `instance_id` Do?

`instance_id` is just a **label (number)** that says:

**"These nodes are following THIS routing strategy."**

### Why Unique `instance_id` is Needed

Imagine two strategies running at the same time:

**Strategy A:**
- Focus: Reliability
- Uses ETX heavily
- instance_id = 0

**Strategy B:**
- Focus: Low latency
- Uses hop count more
- instance_id = 1

Even though:
- Same root
- Same nodes
- Same physical network

👉 The routing decisions are **different**, so RPL keeps them separate using `instance_id`.

Nodes must not mix:
- parents
- ranks
- control messages

That's why each strategy needs its own unique `instance_id`.

### What Happens Inside One `instance_id`?

Inside one `instance_id`:

- There can be **many paths**
- Nodes may **switch parents**
- **Backup parents** exist
- Ranks change **dynamically**

But **all of this follows the same rules**.

### Very Simple Analogy

**instance_id = rulebook**

| Situation | Explanation |
|-----------|-------------|
| Same rulebook | Same instance_id |
| Different rulebooks | Different instance_id |

You can:
- Use many roads (paths)
- But if **rules change** → you need a **new rulebook**

### Why You Usually See `instance_id: 0` in Wi-SUN

- Wi-SUN typically runs **only one routing strategy**
- So only **instance 0** is used
- Multiple instances are **rare and advanced**

### One-Line Takeaway

👉 **`instance_id` does NOT represent different paths.**  
👉 **It represents different routing rules (strategies).**  
👉 **Each strategy gets a unique instance_id.**

---

## Which Routing Strategy Does Wi-SUN Use?

### Short Answer (Most Important)

👉 **Wi-SUN uses a single RPL instance (instance_id = 0) with the `MRHOF` routing strategy (ETX-based).**

### Slightly Longer, Still Simple 🙂

#### 1️⃣ What Routing Strategy Does Wi-SUN Use?

Wi-SUN normally uses:

**MRHOF (Minimum Rank with Hysteresis Objective Function)**

This means:

- The node chooses its parent based mainly on **link quality**
- Link quality is measured using **ETX**

You already see this in your output:

```json
"etx": 138,
"rpl_rank": 258
```

So yes — **ETX is actively used**, which confirms MRHOF.

#### 2️⃣ Why is `instance_id` Always 0?

Because:

- Only **one routing strategy** is running
- No parallel strategies (like low-latency vs low-power)
- So the default and only instance is **ID = 0**

Think of it as:

**"Default routing mode"**

#### 3️⃣ Are There Other Possible Strategies?

Yes, in theory:

| Strategy | Purpose | Used in Wi-SUN? |
|----------|---------|-----------------|
| OF0 (Hop-count) | Shortest path | ❌ Rare |
| MRHOF (ETX) | Best link quality | ✅ Yes (default) |
| Multiple instances | Advanced setups | ❌ Almost never |

Wi-SUN standard deployments stick to **MRHOF only**.

### One Clear Sentence (Final)

👉 When you see:

```json
"instance_id": 0
```

It means:

**The node is using the default Wi-SUN RPL routing strategy: MRHOF (ETX-based), and no other routing instance exists.**

---

## Practical Examples from Your Network

### Example Output: Node RPL Information

**Command:**
```bash
coap-client-notls -m get coap://[fd12:3456::eae:5fff:fe52:69de]:5683/status/my_rank
```

**Output:**
```json
{
  "dodag_rank": 384,
  "instance_id": 0
}
```

**What this tells you:**
- Node is using routing instance 0 (standard Wi-SUN)
- Node's rank is 384 (approximately 2 hops from root)
- Using MRHOF strategy

### Example Output: Neighbor Information

**Command:**
```bash
coap-client-notls -m post coap://[fd12:3456::eae:5fff:fe52:69de]:5683/status/neighbor -e "0"
```

**Output:**
```json
{
  "tag": "9d54",
  "index": 0,
  "type": 0,
  "lifetime": 2200,
  "mac_tx_count": 14,
  "mac_tx_failed_count": 0,
  "mac_tx_ms_count": 0,
  "mac_tx_ms_failed_count": 0,
  "rpl_rank": 128,
  "etx": 160,
  "rsl_in": -75,
  "rsl_out": -38,
  "is_lfn": 0
}
```

**What this tells you:**
- Parent node (type=0) has rank 128
- ETX to parent is 160
- Your rank (384) = Parent rank (128) + ETX (160) + routing cost
- This confirms MRHOF is using ETX for path selection

### Example Output: Second Neighbor

**Command:**
```bash
coap-client-notls -m post coap://[fd12:3456::eae:5fff:fe52:69de]:5683/status/neighbor -e "1"
```

**Output:**
```json
{
  "tag": "4fe1",
  "index": 1,
  "type": 1,
  "lifetime": 2196,
  "mac_tx_count": 21,
  "mac_tx_failed_count": 0,
  "mac_tx_ms_count": 0,
  "mac_tx_ms_failed_count": 0,
  "rpl_rank": 256,
  "etx": 132,
  "rsl_in": -55,
  "rsl_out": -56,
  "is_lfn": 0
}
```

**What this tells you:**
- This is a secondary neighbor (type=1, not parent)
- Has rank 256, ETX 132
- Could be used as backup parent

---

## Key Takeaways

### Quick Reference Table

| Concept | Simple Meaning | Technical Term |
|---------|----------------|----------------|
| instance_id | Which routing map/strategy | RPL Instance Identifier |
| dodag_rank | Distance from root | Logical position in DODAG |
| ETX | Link quality (lower = better) | Expected Transmission Count |
| MRHOF | Wi-SUN's routing strategy | Minimum Rank Hysteresis OF |
| type=0 | Parent node | Primary parent |
| type=1 | Backup/neighbor | Secondary parent |

### Final Summary

✅ **`instance_id`** = which routing strategy (map) is used  
✅ **`dodag_rank`** = how far you are from the root in that map  
✅ **Multiple paths** do NOT imply multiple `instance_id`s  
✅ **Different routing rules** (strategies) create different `instance_id`s  
✅ **Wi-SUN** normally uses `instance_id = 0` with **MRHOF (ETX-based)**  
✅ **ETX values** in neighbor info confirm MRHOF is active  
✅ **Rank calculation**: Your rank ≈ Parent rank + ETX to parent

---

## Related Documentation

For more details on CoAP endpoints and network monitoring, see:
- [COAP-INFO.md](COAP-INFO.md) - Complete CoAP API reference

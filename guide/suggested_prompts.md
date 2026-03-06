### Suggested AI Prompts for Deep Project Exploration

Beyond basic onboarding, these prompts allow a developer to dig into the technical core, safety, and future potential of a codebase.

---

### 1. System Dependency & Coupling Analysis
**Goal**: Understand which parts of the code are "hot spots" and how changes ripple through the system.

**Prompt Template**:
> Map the dependencies between the major subsystems of this project.
> 1. Identify which subsystems are "Core" (others depend on them) and which are "Leaf" (they depend on others).
> 2. Create a list of the top 10 most critical functions or structs that, if modified, would require changes in 5 or more other files.
> 3. Identify any "circular dependencies" where two systems depend on each other, and suggest how to break them to improve modularity.

---

### 2. Memory Layout & Data Structure Deep-Dive
**Goal**: Understand how data is stored in memory, which is essential for performance and porting.

**Prompt Template**:
> Analyze the memory layout and data structures used in the [Subsystem, e.g., Renderer].
> 1. Explain the purpose and layout of the 3 most important structs in this module.
> 2. How are these structs aligned? Are there any padding issues or memory-efficiency "gotchas"?
> 3. Does the system use specific patterns like "SoA" (Structure of Arrays) or "AoS" (Array of Structures)? Why was this chosen?
> 4. Identify where the most expensive memory accesses happen (e.g., pointer-chasing in a linked list vs. contiguous array access).

---

### 3. Concurrency & Multi-threading Readiness
**Goal**: Assess how hard it would be to make a legacy codebase run on multiple CPU cores.

**Prompt Template**:
> Evaluate this project for multi-threading readiness.
> 1. Identify the primary "bottleneck" loop (e.g., the rendering loop or physics ticker).
> 2. What global state is modified inside this loop? This is a "race condition" risk.
> 3. Could this loop be parallelized using Data Parallelism (e.g., processing different pixels/objects on different threads)?
> 4. What would be the most "thread-safe" way to encapsulate the global state to allow for parallel execution without massive lock contention?

---

### 4. Error Handling & Memory Safety Audit
**Goal**: Find potential bugs, crashes, and security vulnerabilities.

**Prompt Template**:
> Perform a safety and error-handling audit of the [Subsystem].
> 1. How does the code handle "out of memory" or "file not found" errors? Is it consistent?
> 2. Identify potential memory safety risks:
>    - Unchecked buffer writes.
>    - Unsafe pointer arithmetic.
>    - Assumptions about input data size (e.g., loading from a WAD/File).
> 3. Suggest a modern "defensive programming" pattern that could be applied to these critical sections without killing performance.

---

### 5. Build System & Toolchain Modernization
**Goal**: Improve the developer experience and portability of the build.

**Prompt Template**:
> Analyze the current build system (e.g., Makefile, CMake).
> 1. Is the build truly cross-platform, or does it rely on local paths/hardcoded tools?
> 2. Identify "Magic Flags": What do the various compiler flags (e.g., `-O3`, `-fomit-frame-pointer`) do for this specific project?
> 3. Recommend a modern CI/CD pipeline setup for this project. What tests should be run on every commit (Unit tests, Sanity checks, Build verification)?

---

### 6. Extensibility & Modding Hooks
**Goal**: Identify how the engine was designed to be extended (or how it could be).

**Prompt Template**:
> Analyze the extensibility of the engine.
> 1. Where are the "hooks" that allow new content (e.g., new monsters, maps, textures) to be added without changing the core engine code?
> 2. Is there a "Scripting" or "Data-Driven" component? If not, identify the best place to insert one (e.g., Lua or a simple VM).
> 3. How does the project handle "Plugins" or "Dynamic Loading" of libraries?

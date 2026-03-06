### Useful AI Prompts for Project Analysis & Onboarding

These prompts have been refined and improved based on our successful analysis of the DOOM engine. They are designed to help a developer quickly understand, document, and plan modifications for a complex, existing codebase.

---

### 1. Project Onboarding & Learning Map
**Goal**: Get a high-level view of a new codebase and identify where to start reading.

**Prompt Template**:
> Analyze this [Programming Language] project as a learning guide. My goal is to understand the high-level architecture, the main execution pipeline, and identify 5 key files or functions to study in order.
>
> Please provide:
> 1. A short map of the project: main subsystems and their responsibilities.
> 2. The runtime flow from program start to [Major Event/Frame/Output].
> 3. For the core logic (e.g., [Subsystem]), describe the pipeline step-by-step:
>    - Initialization
>    - Main loop/setup
>    - Processing stages
>    - Final output
> 4. For each important function/file identified, say:
>    - What problem it solves.
>    - What inputs or global state it depends on.
>    - What global data it reads or writes.
>    - What function is typically called next.
> 5. Call out [Language]-specific patterns that are important for learning (e.g., [Pattern 1], [Pattern 2]).
> 6. Identify if parts of the code are core logic, portability layers, or historical baggage.

**Why this works**: It forces the AI to look at both the "what" (architecture) and the "how" (runtime flow), providing a concrete path for a human to follow.

---

### 2. Strategic Refactoring & Modernization
**Goal**: Identify technical debt and plan how to upgrade a project to modern standards.

**Prompt Template**:
> This project was written in [Year/Standard] and needs to be modernized for [Modern Standard/Environment].
>
> Create a strategic refactoring proposal. Do not focus on line-by-line changes, but rather on high-level structural and architectural improvements.
>
> For each point in your proposal, include:
> 1. **Description of the change**: What should be replaced or updated.
> 2. **Historical Rationale**: Why was it implemented the original way (assumed or known)?
> 3. **Modern Benefit**: Why will this change make the code better (e.g., performance, safety, maintainability)?
>
> Focus on areas like [Memory Management, Type Safety, Parallelization, Library Abstraction].

**Why this works**: It provides the context ("why") behind old code, which prevents developers from making "naive" refactors that break hidden dependencies.

---

### 3. Porting Feasibility & Hardware Adaptation
**Goal**: Assess the effort and create a roadmap for moving code to a new platform (e.g., Embedded, Mobile, Cloud).

**Prompt Template**:
> I want to port this project to [Target Platform/Hardware] (e.g., STM32, WASM, Serverless).
>
> 1. **Difficulty Estimation**: Provide a high-level estimate of the difficulty and the primary constraints (e.g., RAM, I/O, CPU architecture).
> 2. **Refactoring Plan**: Create a detailed, step-by-step list of what needs to be done.
>    - How to isolate the platform-specific layer (I/O, Video, Sound).
>    - How to handle memory constraints (Heap vs. Stack, Custom Allocators).
>    - How to abstract the filesystem or external resource loading.
>    - How to handle hardware synchronization (Timers, Interrupts).
> 3. **Validation Strategy**: How should I test that the core logic is still working before the new hardware drivers are ready?

**Why this works**: It breaks down a massive "Porting" task into logical modules, allowing for parallel development or staged implementation.

---

### 4. Language-Specific Organizational Analysis
**Goal**: Understand the project layout and how it solves language-level limitations.

**Prompt Template**:
> I am coming from a [Language A] background and am new to [Language B] (e.g., Java to C).
>
> 1. Analyze the project's directory structure. Is this a traditional layout for this language? Explain the pros/cons of this organization.
> 2. How does this project handle [Language Feature, e.g., Namespaces, Visibility, Exception Handling] without native support?
> 3. Identify common idioms used in this codebase that might be confusing to someone from [Language A].

**Why this works**: It bridges the "mental gap" between different programming paradigms.

---

### 5. Naming Convention & Domain Lexicon Extraction
**Goal**: Decode the "slang" and naming rules used by the original developers.

**Prompt Template**:
> Analyze the naming conventions and identifier patterns used in this project.
>
> 1. **Convention Review**: What is good, bad, or inconsistent about the current naming style (Variables, Functions, Structs, Macros)?
> 2. **Prefix/Namespace Map**: Identify the 1-3 letter prefixes used to denote subsystems and what they stand for.
> 3. **Domain Dictionary**: Create a dictionary of common nouns, verbs, and abbreviations found in the code.
>    - Extract words from names used for variables, functions, and structures.
>    - Decode cryptic shorthand (e.g., 2-4 letter abbreviations).
>    - Categorize them into Subsystems, Domain Objects, Actions, and Technical Slang.

**Why this works**: It turns "DC_YL" into "Draw Column Y Low," making the code immediately more readable for a non-expert.

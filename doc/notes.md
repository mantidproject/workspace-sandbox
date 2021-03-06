Key problems to solve:
- Link various information together, including instrument, logs, related workspaces, masking.
- How can we write generic algorithms without a stiff inheritance tree. Are templates the right way?
  - How can we write "algorithm" code that does not need to know about the ADS, properties, type conversions?

- Would it make sense to a have separate ADS section for each workspace type, such that casting can be avoided?
- The guiding design principle should always be ease of use (intuitive, etc.).
- Workspaces should support chaining multiple algorithms calls, to improve cache use.
- Workspace types are mirrored in property types? If we add new workspaces we also need to add new properties, unless all workspaces inherit from the same class?

TODO:
- implications on property system
- stitching
- ops between worksapaces of different shape / with different spectrum content

# Draft 1

If we do not have an inheritance tree for workspaces, how can we write generic algorithms?
`draft1.cpp` shows a way, client algorithm code is very simple.

- Will this get cumbersome with many properties?
- Do we need a property system at the algorithm implementation level?
- Do we actually need the ADS in C++?

# Draft 2

No ADS in C++. pybind11 and Python do everything for us?

# Draft 3

- Using `boost::any` is probably just a simpler variant of Draft-1, but runs into the same problem.
- How can we make any overloaded function into an algorithm? Do we need to?
- Should we write algorithms always in two parts, a shell that deals with properties, and normal C++ code doing the actual work?
  - Is making this simple part of the workspace scope? My current feeling is that getting the workspace design wrong could make this difficult on the algorithm side, so I believe we must at the very least come up with a working proof of concept for this.

# Draft 4

- What do algorithms actually need? Most do not need, e.g., instrument, or `SpectrumInfo`. Table of L2, etc. might be enough.
  - See for example discussion at developer meeting on `ConvertUnits` not working for constant-wavelength instruments -- only wavelength is needed.
  - Can we select the right overload depending on what information is available?
- We have to avoid duplicating concepts.
  - Example1: Masking. Currently there is `MaskWorkspace` as well as masking on the instrument/workspace.
  - Can we eliminate one of these? Either masking should always be stored in workspace, or always we a separate workspace (provided to algorithms by hand, or used automatically when linked).
  - Same for `GroupingWorkspace`: We have grouping information in the workspace, and we have workspaces defining grouping patterns. Eliminate one.
  - If we eliminate the workspaces, provide a different way for users to view that masking/grouping information.
- Should the instrument be handled in the same way as discussed for masking and grouping above?
- How do we modify such linked workspaces, such as `MaskWorkspace`?
  - If directly, should it always takes a copy (break all links)? This would add overhead of copying, and later setting a new link from data workspace to the modified mask workspace.
  - If directly but not copy, how can we possibly avoid breaking data workspaces that link?
  - If via link, take a copy (if necessary, there might just be a single link), link stays intact, but the flow of logic is not nice (modify one object via another)
  - Forbid changing grouping in existing workspace? This does usually not make sense.
- Are the `Axis` objects duplicating information that is contained in `Histogram` and the spectrum numbers (if we generalize to more generic spectrum labels)? What happens on transpose? X might need generic labels (actually `Transpose` does not support that!)?

# Iterator (`iterator.cpp`)

- Iterators that provide a unified view of information linked to by workspace, including data, spectrum labels, spectrum definitions, masking.
  - Low-level algorithms to work with iterators?
- If we link to workspaces from iterators, access should most likely be only `const` (never modify linked workspaces via these iterators, there are *data* iterators).
- Workspaces are "the same" for some algorithms but different types for others.
  - For example, `Rebin` can work on anything that contains histograms, but histograms could be linked to detector pixels (positions), a two-theta value, Q, or nothing.
  - An algorithm like `ConvertUnits` does need to handle these differently.
  - Can we and should we express this via the type system?
  - One way is at the iterator level. Open questions: How to create output workspace. Whichever piece of code gets the iterators needs to know the actual type, unless we use inheritance to handle this?
  - Is a better way via a view ("overlay")?
  - Inheritance?
  - Algorithms should just run on the relevant sub-object!

# Composition (`composition.cpp`)

- Shows how to write algorithms for sub-components.
- Why not have only one workspace type and check if certain sub fields exist?
  - Would lead to messy client code, e.g., `ConvertUnits`:
    ```
    if (ws.hasSpectrumInfo())
      convertUnits(ws.histograms(), ws.spectrumInfo());
    // FIXME make sure we don't have SpectrumInfo as well as wavelength!
    if (ws.hasFixedWavelength()) // reactors
      convertUnits(ws.histograms(), ws.wavelength());
    else
      throw std::runtime_error("No information for converting units in workspace");
    ```

# Composition 2 (`composition2.cpp`)

Key problem to solve: If we have many more workspace types, we need a way of branching in algorithms.
The current dynamic casting of the property to EventWorkspace or Workspace2D does not scale!
If we pass workspaces via a property we cannot use templates.
- Is this actually a problem? How many distinct data types are there?
- Worst case might be something like scale, which works for many types:
  - Histograms
  - EventLists
  - constant-wavelength data
  - single data points
  - POD types like double or int
- Most algorithms cover fewer cases?
- Can a simple helper function that does the mapping from type id to type do the trick?

# Structure-of-arrays / array-of-structures abstraction (`soa_aos_abstraction.cpp`)

It would be good if we could hide the implementation details of a workspace.
Client code should not need to know whether the workspace contains a vector of structures containing all components for each index, or a number of separate vectors for each component.

If we provide array-of-structures access, there may be a number of problems:
- How can we validate data across the workspace, such as when setting a spectrum numbers.
  - Allow non-unique spectrum numbers (makes lookup via spectrum numbers problematic).
  - Make spectrum numbers immutable via workspace items?
  We have a similar problem if we want to ensure the no two spectra map to the same detector.
- How can we filter/slice and merge workspaces, without generating an inconsistent state?
  - What guarantees do we need to make?
  - In item assignment or `push_back` into a workspace we can check consistency of spectrum numbers, against a local map (local on each rank in an MI run).
    Keeping a global map up to date would be too expensive and a per-item level with every `push_back`.
    If we need to use spectrum numbers for item lookup we would thus need to update the maps (spectrum number and global spectrum index) globally.
    For lookup happening when calling an algorithm this might be an acceptable overhead (although incurring overhead and unnecessary synchronization even if there are no changes).
    The way `IndexInfo` turned out does not feel like the right thing, so maybe this is indicating that another way, as discussed here, might be a better solution.
- If data is mutable via workspace items, this would currently support changing histogram types and sizes.
  Changing sizes might not be an issue since this is on the way of being supported anyway.
  Changing histogram types, e.g., from point data to histogram data would currently be more problematic, but actually we do not have such checks in place anyway.
  The best solution might be to split `Histogram` into actual types instead of having a chimera.
- If masking, grouping, etc. are provided via linked workspaces, how do we handle assignment?
  We want to share the data.
  Just COW if using a non-const iterator?
  We may want to assign data but keep everything else, might need different iterator types for this.

A SOA/SAO abstraction could also pave the way for better imaging support.
Internally a workspace could store data as a stack of images (one for each TOF).
But maybe not...:
- Not clear how to access TOF histogram, need to add stride support in `Histogram`?
- Would be very inefficient to access as histogram.


## Aspects I really like

1. The use of univeral references to determine wheter the operations can take place in-place or not. Looks like standard c++ move semantics from the algorithm client's perspective.
1. Doing away with Properties, in favour of usual c++ function arguments. Looks like standard c++ from the algorithm author and callers perspective.
1. Aligned with the above, better compile-time checking and type safety around combining algoithms with workspaces, and making calls to them.
1. I like the overall generic workspace exposing iterators for accessing the data.
```cpp
template <typename DataType>
class Workspace{
  ... 
  std::vector<DataType>
};
```

## Questions, Clarifications, Investigations

1. I think we are currently restricted to only be able to iterate over the data. What about if we needed information from each corresponding `SpectrumDefinition` as well within each `apply` call. Currently there is no way to do this.

2. The `Workspace` you have suggested is a good fit for `MatrixWorkspace` type workspaces. If it were not for the `SpectrumDefinition`s, `SpectrumNumber`s etc, this type of workspace would be very generic and very suitable for storing any type of data.

3. I like the abstraction from the iteration over Data items. However:
    1. I can't see how the current approach supports something like `SumSpectra`. 
    1. I can't see how we would be able to do anything special around monitor spectra in the current implementation
    1. In general, I think you may not be able to get away from offering an `apply(Workspace<T>&& ws, ..)` for some algorithm operations.
    
4. Linked to the above, I think we should also review a historic decision in Mantid. Do we really need to to allocate n*3 vectors, where `n` is the number of spectra. Would it be possible to have 3 vectors per workspace, (better for numpy), but there may be other considerations.

5. Not sure about co-opting Workspace copy constructors like [this](https://github.com/mantidproject/workspace-sandbox/blob/master/src/prototype/workspace.h#L24), this would lead to unexpected results. 
```cpp
auto wsCopy = ws;
```
wsCopy is the right size, but is unitializied.
I think named, explicit member functions would be better for this.

6. I can't see why we need to support anything other than a default constructor? see [here](https://github.com/mantidproject/workspace-sandbox/tree/master/src/call_wrappers.h#L98-L113)

7. Would it be possible to wrap `Workspace<T>` in some of the current `MatrixWorkspace`, or other interfaces so that we can incrementally update Mantid with this design? We would need to consider how such changes could be rolled out.


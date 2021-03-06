#include <vector>
#include <memory>

// Part 1 Views

// Support Workspace2D, EventWorkspace, transpose, masking in child classes?
// Give shape to Y, slice in X?
// Should this be called Model or View?
class HistogramView {
public:
  virtual ~HistogramView() = default;
  virtual Histogram operator[](const size_t i) = 0;
};

// Use this to get rid of MRU in EventWorkspace.
// Algorithms like ConvertUnits and Rebin need to support views!
class EventWorkspaceHistogramView : public HistogramView {
public:
  EventWorkspaceHistogramView(const EventWorkspace data) : m_data(data) {}
  Histogram operator[](const size_t i)override {
    return rebin(m_data[i], m_binEdges[i]);
  }

private:
  const EventWorkspace &m_data;
  // When calling rebin/convertUnits on a workspace, how to we handle
  // rebinning/unit-conversion on data like BinEdges in the attached views?
  // Should BinEdges be part of the data?
  // In general, can views always be preserved across algorithm calls as long as
  // they do not contain data? Yes, if modification is in-place? If they do not
  // contain data, why would we need to attach them to the workspace level, they
  // could just be created on the fly?
  std::vector<BinEdges> m_binEdges;
};

// PanelView -> this is what ILL (SANS?) asked for, but isn't this simply what
// the instrument view provides?

// Part 2 Links / workspace-type vs. workspace-composition redundancy

class GroupingWorkspace {
public:
  const SpectrumDefinition &spectrumDefinition(const size_t i) const {
    return m_spectrumDefinitions[i];
  }

private:
  std::vector<SpectrumDefinition> m_spectrumDefinitions;
};

class Workspace2D {
public:
private:
  std::vector<Histogram> m_histograms;
  const GroupingWorkspace &m_grouping; // Do NOT store grouping anywhere else
                                       // (ISpectrum, IndexInfo, SpectrumInfo)
};

int main() {}

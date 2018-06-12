#include <gtest/gtest.h>

#include <boost/mpl/at.hpp>
#include <boost/mpl/sort.hpp>
#include <boost/mpl/vector_c.hpp>

#include "test_macros.h"

#include "dataset_view.h"

TEST(MultidimensionalIndex, end) {
  const std::vector<gsl::index> volume{3, 1, 2};
  MultidimensionalIndex i(volume);
  ASSERT_EQ(i.end, (std::vector<gsl::index>{2, 0, 1}));
}

TEST(MultidimensionalIndex, increment) {
  const std::vector<gsl::index> volume{3, 1, 2};
  MultidimensionalIndex i(volume);
  ASSERT_EQ(i.index, (std::vector<gsl::index>{0, 0, 0}));
  i.increment();
  ASSERT_EQ(i.index, (std::vector<gsl::index>{1, 0, 0}));
  i.increment();
  ASSERT_EQ(i.index, (std::vector<gsl::index>{2, 0, 0}));
  i.increment();
  ASSERT_EQ(i.index, (std::vector<gsl::index>{0, 0, 1}));
  i.increment();
  ASSERT_EQ(i.index, (std::vector<gsl::index>{1, 0, 1}));
  i.increment();
  ASSERT_EQ(i.index, (std::vector<gsl::index>{2, 0, 1}));
}

TEST(LinearSubindex, full_subindex) {
  const std::vector<gsl::index> volume{3, 1, 2};
  const std::map<Dimension, gsl::index> extents{
      {Dimension::Tof, 3}, {Dimension::SpectrumNumber, 1}, {Dimension::Q, 2}};
  MultidimensionalIndex i(volume);
  Dimensions dims;
  dims.add(Dimension::Tof, 3);
  dims.add(Dimension::SpectrumNumber, 1);
  dims.add(Dimension::Q, 2);

  LinearSubindex sub(extents, dims, i);
  gsl::index count{0};
  while (true) {
    ASSERT_EQ(sub.get(), count++);
    if (i.index == i.end)
      break;
    i.increment();
  }
}

TEST(LinearSubindex, zero_dimensional_subindex) {
  const std::vector<gsl::index> volume{3, 1, 2};
  const std::map<Dimension, gsl::index> extents{
      {Dimension::Tof, 3}, {Dimension::SpectrumNumber, 1}, {Dimension::Q, 2}};
  MultidimensionalIndex i(volume);

  LinearSubindex sub(extents, Dimensions{}, i);
  ASSERT_EQ(sub.get(), 0);
  i.increment();
  ASSERT_EQ(sub.get(), 0);
  i.increment();
  ASSERT_EQ(sub.get(), 0);
  i.increment();
  ASSERT_EQ(sub.get(), 0);
  i.increment();
  ASSERT_EQ(sub.get(), 0);
  i.increment();
  ASSERT_EQ(sub.get(), 0);
}

TEST(LinearSubindex, fast_1_dimensional_subindex) {
  const std::vector<gsl::index> volume{3, 1, 2};
  const std::map<Dimension, gsl::index> extents{
      {Dimension::Tof, 3}, {Dimension::SpectrumNumber, 1}, {Dimension::Q, 2}};
  MultidimensionalIndex i(volume);

  LinearSubindex sub(extents, Dimensions(Dimension::Tof, 3), i);
  ASSERT_EQ(sub.get(), 0);
  i.increment();
  ASSERT_EQ(sub.get(), 1);
  i.increment();
  ASSERT_EQ(sub.get(), 2);
  i.increment();
  ASSERT_EQ(sub.get(), 0);
  i.increment();
  ASSERT_EQ(sub.get(), 1);
  i.increment();
  ASSERT_EQ(sub.get(), 2);
}

TEST(LinearSubindex, slow_1_dimensional_subindex) {
  const std::vector<gsl::index> volume{3, 1, 2};
  const std::map<Dimension, gsl::index> extents{
      {Dimension::Tof, 3}, {Dimension::SpectrumNumber, 1}, {Dimension::Q, 2}};
  MultidimensionalIndex i(volume);

  LinearSubindex sub(extents, Dimensions(Dimension::Q, 2), i);
  ASSERT_EQ(sub.get(), 0);
  i.increment();
  ASSERT_EQ(sub.get(), 0);
  i.increment();
  ASSERT_EQ(sub.get(), 0);
  i.increment();
  ASSERT_EQ(sub.get(), 1);
  i.increment();
  ASSERT_EQ(sub.get(), 1);
  i.increment();
  ASSERT_EQ(sub.get(), 1);
}

TEST(LinearSubindex, flipped_2_dimensional_subindex) {
  const std::vector<gsl::index> volume{3, 1, 2};
  const std::map<Dimension, gsl::index> extents{
      {Dimension::Tof, 3}, {Dimension::SpectrumNumber, 1}, {Dimension::Q, 2}};
  MultidimensionalIndex i(volume);
  Dimensions dims;
  dims.add(Dimension::Q, 2);
  dims.add(Dimension::Tof, 3);

  LinearSubindex sub(extents, dims, i);
  ASSERT_EQ(sub.get(), 0);
  i.increment();
  ASSERT_EQ(sub.get(), 2);
  i.increment();
  ASSERT_EQ(sub.get(), 4);
  i.increment();
  ASSERT_EQ(sub.get(), 1);
  i.increment();
  ASSERT_EQ(sub.get(), 3);
  i.increment();
  ASSERT_EQ(sub.get(), 5);
}

TEST(DatasetView, construct) {
  Dataset d;
  d.insert<Variable::Value>("name1", Dimensions{}, {1.1});
  d.insert<Variable::Int>("name2", Dimensions{}, {2l});
  ASSERT_NO_THROW(DatasetView<> view(d));
  ASSERT_NO_THROW(DatasetView<Variable::Value> view(d));
  ASSERT_NO_THROW(DatasetView<Variable::Int> view(d));
  ASSERT_NO_THROW(auto view = (DatasetView<Variable::Int, Variable::Value>(d)));
  ASSERT_THROW(auto view = (DatasetView<Variable::Int, Variable::Error>(d)),
               std::runtime_error);
}

TEST(DatasetView, single_column) {
  Dataset d;
  d.insert<Variable::Value>("name1", Dimensions(Dimension::Tof, 10), 10);
  d.insert<Variable::Int>("name2", Dimensions(Dimension::Tof, 10), 10);
  auto &var = d.get<Variable::Value>();
  var[0] = 0.2;
  var[3] = 3.2;

  DatasetView<Variable::Value> view(d);
  ASSERT_EQ(view.get<Variable::Value>(), 0.2);
  view.increment();
  ASSERT_EQ(view.get<Variable::Value>(), 0.0);
  view.increment();
  ASSERT_EQ(view.get<Variable::Value>(), 0.0);
  view.increment();
  ASSERT_EQ(view.get<Variable::Value>(), 3.2);
}

TEST(DatasetView, multi_column) {
  Dataset d;
  d.insert<Variable::Value>("name1", Dimensions(Dimension::Tof, 2), 2);
  d.insert<Variable::Int>("name2", Dimensions(Dimension::Tof, 2), 2);
  auto &var = d.get<Variable::Value>();
  var[0] = 0.2;
  var[1] = 3.2;

  DatasetView<Variable::Value, Variable::Int> view(d);
  ASSERT_EQ(view.get<Variable::Value>(), 0.2);
  ASSERT_EQ(view.get<Variable::Int>(), 0);
  view.increment();
  ASSERT_EQ(view.get<Variable::Value>(), 3.2);
  ASSERT_EQ(view.get<Variable::Int>(), 0);
}

TEST(DatasetView, multi_column_mixed_dimension) {
  Dataset d;
  d.insert<Variable::Value>("name1", Dimensions(Dimension::Tof, 2), 2);
  d.insert<Variable::Int>("name2", Dimensions{}, 1);
  auto &var = d.get<Variable::Value>();
  var[0] = 0.2;
  var[1] = 3.2;

  ASSERT_ANY_THROW(auto view =
                       (DatasetView<Variable::Value, Variable::Int>(d)));
  ASSERT_NO_THROW(auto view =
                      (DatasetView<Variable::Value, const Variable::Int>(d)));
  auto view = (DatasetView<Variable::Value, const Variable::Int>(d));
  ASSERT_EQ(view.get<Variable::Value>(), 0.2);
  ASSERT_EQ(view.get<const Variable::Int>(), 0);
  view.increment();
  ASSERT_EQ(view.get<Variable::Value>(), 3.2);
  ASSERT_EQ(view.get<const Variable::Int>(), 0);
}

TEST(DatasetView, multi_column_unrelated_dimension) {
  Dataset d;
  d.insert<Variable::Value>("name1", Dimensions(Dimension::X, 2), 2);
  d.insert<Variable::Int>("name2", Dimensions(Dimension::Y, 3), 3);
  DatasetView<Variable::Value> view(d);
  view.increment();
  // We iterate only Variable::Value, so there should be no iteration in
  // Dimension::Y.
  ASSERT_TRUE(view.atLast());
}

TEST(DatasetView, multi_column_mixed_dimension_with_slab) {
  Dataset d;
  d.insert<Variable::Value>("name1", Dimensions(Dimension::Tof, 2), 2);
  d.insert<Variable::Int>("name2", Dimensions{}, 1);
  auto &var = d.get<Variable::Value>();
  var[0] = 0.2;
  var[1] = 3.2;

  // Should fixed dimension be generic, or should we just provide a couple of
  // special cases, in particular for Tof?
  // Use direct column access otherwise (how to access things like Tof slices?)?
  // YAGNI? Just support a single fixed dimension, given at compile time?!
  // We might want to iterate all BinEdges, getting a slab of corresponding
  // counts (e.g., all spectra for certain polarization)?! That is,
  // Dimension::Tof and Dimension::SpectrumNumber might be fixed, while
  // Dimension::Polarization is running. Or Dimension::Tof and
  // Dimension::Polarization are fixed file Dimension::SpectrumNumber is
  // running. The latter is more likely to be expressed as "iterate only
  // SpectrumNumber", how do we handle that in general?
  // Maybe a better way to say this is: Iterate all dimensions of BinEdges. In
  // general we do not know which other columns need to be accessed as slabs,
  // how can we deal with this? Just access all as slab (which may be size 1)?
  DatasetView<Slab<Variable::Value>, Variable::Int> view(d, {Dimension::Tof});
  // view.get<double>(); // Does not compile, since we cannot get a single
  // double.
  view.get<Variable::Int>();
}

TEST(DatasetView, single_column_edges) {
  Dataset d;
  auto edges = makeDataArray<Variable::Value>(Dimensions(Dimension::Tof, 3), 3);
  d.insertAsEdge(Dimension::Tof, edges);
  d.insert<Variable::Int>("name2", Dimensions(Dimension::Tof, 2), 2);
  auto &var = d.get<Variable::Value>();
  ASSERT_EQ(var.size(), 3);
  var[0] = 0.2;
  var[2] = 2.2;

  DatasetView<Variable::Value> view(d);
  ASSERT_EQ(view.get<Variable::Value>(), 0.2);
  view.increment();
  ASSERT_EQ(view.get<Variable::Value>(), 0.0);
  ASSERT_FALSE(view.atLast());
  view.increment();
  ASSERT_EQ(view.get<Variable::Value>(), 2.2);
  ASSERT_TRUE(view.atLast());
}

TEST(DatasetView, single_column_bins) {
  Dataset d;
  auto edges = makeDataArray<Variable::Tof>(Dimensions(Dimension::Tof, 3), 3);
  d.insertAsEdge(Dimension::Tof, edges);
  d.insert<Variable::Int>("name2", Dimensions(Dimension::Tof, 2), 2);
  auto &var = d.get<Variable::Tof>();
  ASSERT_EQ(var.size(), 3);
  var[0] = 0.2;
  var[1] = 1.2;
  var[2] = 2.2;

  DatasetView<Bins<Variable::Tof>> view(d);
  view.increment();
  // Lenth of edges is 3, but there are only 2 bins!
  ASSERT_TRUE(view.atLast());
}

TEST(DatasetView, multi_column_edges) {
  Dataset d;
  auto edges = makeDataArray<Variable::Tof>(Dimensions(Dimension::Tof, 3), 3);
  d.insertAsEdge(Dimension::Tof, edges);
  d.insert<Variable::Int>("name2", Dimensions(Dimension::Tof, 2), 2);
  auto &var = d.get<Variable::Tof>();
  var[0] = 0.2;
  var[1] = 1.2;
  var[2] = 2.2;

  // Cannot simultaneously iterate edges and non-edges, so this throws.
  EXPECT_THROW_MSG(
      (DatasetView<Variable::Tof, Variable::Int>(d)), std::runtime_error,
      "One of the variables requested for iteration represents bin edges, "
      "direct joint iteration is not possible. Use the Bins<> wrapper to "
      "iterate over bins defined by edges instead.");

  DatasetView<Bins<Variable::Tof>, Variable::Int> view(d);
  // TODO Singular 'Bin' instead of 'Bins' would make more sense.
  // TODO What are good names for named getters? tofCenter(), etc.?
  const auto &bin = view.get<Bins<Variable::Tof>>();
  EXPECT_EQ(bin.center(), 0.7);
  EXPECT_EQ(bin.width(), 1.0);
  EXPECT_EQ(bin.left(), 0.2);
  EXPECT_EQ(bin.right(), 1.2);
  view.increment();
  ASSERT_TRUE(view.atLast());
}

TEST(DatasetView, named_getter) {
  Dataset d;
  auto tof = makeDataArray<Variable::Tof>(Dimensions(Dimension::Tof, 3), 3);
  d.insert(tof);
  auto &var = d.get<Variable::Tof>();
  ASSERT_EQ(var.size(), 3);
  var[0] = 0.2;
  var[2] = 2.2;

  DatasetView<Variable::Tof> view(d);
  ASSERT_EQ(view.tof(), 0.2);
  view.increment();
  ASSERT_EQ(view.tof(), 0.0);
  view.increment();
  ASSERT_EQ(view.tof(), 2.2);
}

TEST(DatasetView, histogram) {
  Dataset d;
  auto tof = makeDataArray<Variable::Tof>(Dimensions(Dimension::Tof, 3), 3);
  d.insertAsEdge(Dimension::Tof, tof);
  Dimensions dims;
  dims.add(Dimension::Tof, 2);
  dims.add(Dimension::Spectrum, 4);
  d.insert<Variable::Value>("sample", dims,
                            {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0});
  d.insert<Variable::Error>("sample", dims, 8);

  // Implies that Dimension::Tof is fixed.
  // TODO How can we identify which dimension the histogram spans?
  // Histogram<Dimension::Tof>?
  // How to deal with multiple data variables, e.g., monitors which would be
  // present in many cases?
  // Are we forced to (almost) always use a string to identify variables?
  // Handle duplicate matches as extra dimension (assuming only one variable is
  // duplicate)?
  // makeDatasetView(d, Variable::Histogram("sample"));
  // Find variables by their dimensions? E.g, Dimension::Monitor?
  // Does grouping variables help? sample.tof, sample.value, sample.error?
  // Use distinct variable names for monitor data? Will not help in generic
  // cases of Datasets containing multiple data variables, e.g., for sample and
  // can, i.e., all algorithms that want to support generic cases will need to
  // referencing variables by their name anyway?
  DatasetView<Variable::Histogram> view(d);
  EXPECT_EQ(view.get<Variable::Histogram>().value(0), 1.0);
  EXPECT_EQ(view.get<Variable::Histogram>().value(1), 2.0);
  view.get<Variable::Histogram>().value(1) += 0.2;
  EXPECT_EQ(d.get<Variable::Value>()[1], 2.2);
  view.increment();
  EXPECT_EQ(view.get<Variable::Histogram>().value(0), 3.0);
}

#if 0
TEST(DatasetView, notes) {
  Dataset d(std::vector<double>(1), std::vector<int>(1));
  d.addDimension("tof", 10);
  d.extendAlongDimension(ColumnType::Doubles, "tof");
  d.addDimension("spec", 10);
  d.extendAlongDimension(ColumnType::Doubles, "spec");
  d.extendAlongDimension(ColumnType::Ints, "spec");

  // ok
  DatasetView<double> view(d, 0);
  // should throw, because int has less dimensions and is not const
  DatasetView<double, int> view(d, 0);
  // ok
  DatasetView<double, const int> view(d, 0);
  // ok, int can be non-const since slab says "do not iterate tof".
  // This is a common case so we may want a shorthand notation for this.
  // We also need a way to have multiple columns of the same type, X,Y,E of a
  // histogram are all double currently! If we have a type for, e.g., BinEdges
  // wrapping a std::vector<double> we can use that (but template argument would
  // not be item type anymore!). Can we use a column ID instead? Since the
  // iterator is templated we cannot support custom types anyway.
  // Unit handling?? Can/should a slab have a (runtime) unit? Could used by call
  // wrapper to create output workspace (cannot *set* based on it due to
  // conflict with multi threading, but can *check*)? (do we even need call
  // wrappers, or should things be handled based on Dataset and
  // transformations?).
  DatasetView<slab<double, Dimension::Tof>, int> view(d, 0);
  // iterate over items that are slabs of doubles with a spectrum dimension. In
  // this case iteration is over tof.
  // A slab is a bit like gsl::range, i.e., a view into a vector but may have a
  // spread to support multi-dimensional slices.
  DatasetView<slab<double, Dimension::Spectrum>,
                  slab<const int, Dimension::Spectrum>> view(d, 0);
}
#endif

template <class T> constexpr int type_to_id();
template <> constexpr int type_to_id<double>() { return 0; }
template <> constexpr int type_to_id<int>() { return 1; }
template <> constexpr int type_to_id<char>() { return 2; }

template <int N> struct id_to_type;
template <> struct id_to_type<0> { using type = double; };
template <> struct id_to_type<1> { using type = int; };
template <> struct id_to_type<2> { using type = char; };
template <int N> using id_to_type_t = typename id_to_type<N>::type;

template <class Sorted, size_t... Is>
auto sort_types_impl(std::index_sequence<Is...>) {
  return std::tuple<
      id_to_type_t<boost::mpl::at_c<Sorted, Is>::type::value>...>{};
}

template <class... Ts> auto sort_types() {
  using Unsorted = boost::mpl::vector_c<int, type_to_id<Ts>()...>;
  return sort_types_impl<typename boost::mpl::sort<Unsorted>::type>(
      std::make_index_sequence<sizeof...(Ts)>{});
}

// Named "Set" because the order of types in the declaration does not matter,
// yields the same type.
template <class... Ts> using Set = decltype(sort_types<Ts...>());

TEST(SortTypes, same) {
  using unsorted1 = boost::mpl::vector_c<int, 4, 3, 1>;
  using unsorted2 = boost::mpl::vector_c<int, 4, 1, 3>;
  ASSERT_EQ(typeid(boost::mpl::sort<unsorted1>::type),
            typeid(boost::mpl::sort<unsorted2>::type));
}

TEST(SortTypes, different) {
  using unsorted1 = boost::mpl::vector_c<int, 4, 3, 1>;
  using unsorted2 = boost::mpl::vector_c<int, 4, 1, 2>;
  ASSERT_NE(typeid(boost::mpl::sort<unsorted1>::type),
            typeid(boost::mpl::sort<unsorted2>::type));
}

TEST(SortTypes, sort) {
  auto t = sort_types<char, double, int>();
  ASSERT_EQ(typeid(decltype(t)), typeid(std::tuple<double, int, char>));
}

TEST(SortTypes, type) {
  Set<char, double, int> a;
  Set<double, char, int> b;
  ASSERT_EQ(typeid(decltype(a)), typeid(decltype(b)));
}
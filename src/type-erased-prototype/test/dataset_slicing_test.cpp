#include <gtest/gtest.h>

#include "test_macros.h"

#include "dataset.h"
#include "dataset_view.h"
#include "dimensions.h"

TEST(Dataset, create_Variable_from_DatasetView) {
  Dataset d;
  d.insert<Data::Value>("name", Dimensions{Dimension::X, 2}, {1.1, 2.2});

  DatasetView<Data::Value> view(d);
  ASSERT_NO_THROW(Variable(tag_id<Data::Value>, Data::Value::unit,
                           Dimensions{Dimension::X, 2}, view));
  Variable var(tag_id<Data::Value>, Data::Value::unit,
               Dimensions{Dimension::X, 2}, view);
  // Would need to make this work with the default get(), otherwise we also run
  // into trouble in DatasetView.
  // Store flag in Dataset, indicated that it is just a view?
  EXPECT_EQ(var.getView<Data::Value>().size(), 2);
  
  Dataset slice;
  slice.insert(var);
  // Would be better to instantiate this with the correct subdimension, instead
  // of dealing with a view of a view. Would avoid expensive setIndex() calls on
  // inner view, operating directly on array of data.
  // Should the viewing Dataset contain a MultiIndex (or VariableView?) instead
  // of a DatasetView?
  // How to control COW?
  // Potentially multiple slices, if we do not trigger COW upon slicing, doing
  // so later will break other slices?
  // =>
  // a) read/write slices trigger COW for *all* variables
  // b) read only slices? (would still break if we trigger COW on root Dataset?)
  // c) just make a copy (*better* than triggering COW for all, will not cause
  //    copy of anything that is constant along the slicing dimension.)
  Dataset slice1 = slice(d, Dimension::X, 1.1);
  Dataset slice2 = slice(d, Dimension::X, 2.2);
  DatasetView<Data::Value> sliceView(slice);
}

TEST(Dataset, insert_DatasetView) {
  //Dataset d;
  //d.insert<Data::Value>("name1", Dimensions{}, {1.1});
  //d.insert<Data::Int>("name2", Dimensions{}, {2l});
  //EXPECT_THROW_MSG(d.insert<Data::Int>("name2", Dimensions{}, {2l}),
  //                 std::runtime_error,
  //                 "Attempt to insert data of same type with duplicate name.");
  //ASSERT_EQ(d.size(), 2);
}

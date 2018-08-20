#ifndef VARIABLE_VIEW_H
#define VARIABLE_VIEW_H

#include <boost/iterator/iterator_facade.hpp>

#include "dimensions.h"
#include "multi_index.h"

template <class T> class VariableView {
public:
  VariableView(const T &variable, const Dimensions &targetDimensions,
               const Dimensions &dimensions)
      : m_variable(variable), m_targetDimensions(targetDimensions),
        m_dimensions(dimensions) {
    if (!targetDimensions.contains(dimensions))
      throw std::runtime_error("Requested target dimensions to not contain "
                               "the variable's dimensions.");
  }

  class iterator
      : public boost::iterator_facade<iterator, const typename T::value_type,
                                      boost::random_access_traversal_tag> {
  public:
    iterator(const T &variable, const Dimensions &targetDimensions,
             const Dimensions &dimensions, const gsl::index index)
        : m_variable(variable), m_index(targetDimensions, {dimensions}) {
      m_index.setIndex(index);
    }

  private:
    friend class boost::iterator_core_access;

    bool equal(const iterator &other) const { return m_index == other.m_index; }
    void increment() { m_index.increment(); }
    const auto &dereference() const { return m_variable[m_index.get<0>()]; }
    void decrement() { m_index.setIndex(m_index.index() - 1); }
    void advance(int64_t delta) {
      if (delta == 1)
        increment();
      else
        m_index.setIndex(m_index.index() + delta);
    }
    int64_t distance_to(const iterator &other) const {
      return static_cast<int64_t>(other.m_index.index()) -
             static_cast<int64_t>(m_index.index());
    }

    const T &m_variable;
    MultiIndex m_index;
  };

  iterator begin() const {
    return {m_variable, m_targetDimensions, m_dimensions, 0};
  }
  iterator end() const {
    return {m_variable, m_targetDimensions, m_dimensions,
            m_targetDimensions.volume()};
  }

private:
  const T &m_variable;
  const Dimensions m_targetDimensions;
  const Dimensions m_dimensions;
};

#endif // VARIABLE_VIEW_H
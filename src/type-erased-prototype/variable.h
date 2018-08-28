#ifndef VARIABLE_H
#define VARIABLE_H

#include <string>
#include <type_traits>

#include <gsl/gsl_util>
#include <gsl/span>

#include "cow_ptr.h"
#include "dimensions.h"
#include "tags.h"
#include "unit.h"

class VariableConcept {
public:
  virtual ~VariableConcept() = default;
  virtual std::unique_ptr<VariableConcept> clone() const = 0;
  virtual bool operator==(const VariableConcept &other) const = 0;
  virtual VariableConcept &operator+=(const VariableConcept &other) = 0;
  virtual VariableConcept &operator*=(const VariableConcept &other) = 0;
  virtual gsl::index size() const = 0;
  virtual void resize(const gsl::index) = 0;
  virtual void copyFrom(const VariableConcept &other, const Dimension dim,
                        const gsl::index offset) = 0;

  virtual const Dimensions &dimensions() const = 0;
  virtual void setDimensions(const Dimensions &dimensions) = 0;
};

class Variable {
public:
  template <class T>
  Variable(uint32_t id, const Unit::Id unit, Dimensions dimensions, T object);

  const std::string &name() const { return m_name; }
  void setName(const std::string &name) {
    if (isCoord())
      throw std::runtime_error("Coordinate variable cannot have a name.");
    m_name = name;
  }
  bool operator==(const Variable &other) const;
  Variable &operator+=(const Variable &other);
  Variable &operator*=(const Variable &other);

  const Unit &unit() const { return m_unit; }
  void setUnit(const Unit &unit) {
    // TODO
    // Some variables are special, e.g., Data::Tof, which must always have a
    // time-of-flight-related unit. We need some sort of check here. Is there a
    // better mechanism to implement this that does not require gatekeeping here
    // but expresses itself on the interface instead? Does it make sense to
    // handle all unit changes by conversion functions?
    m_unit = unit;
  }

  gsl::index size() const { return m_object->size(); }

  const Dimensions &dimensions() const { return m_object->dimensions(); }
  void setDimensions(const Dimensions &dimensions) {
    m_object.access().setDimensions(dimensions);
  }

  const VariableConcept &data() const { return *m_object; }
  VariableConcept &data() { return m_object.access(); }

  template <class Tag> bool valueTypeIs() const {
    return tag_id<Tag> == m_type;
  }

  uint16_t type() const { return m_type; }
  bool isCoord() const { return m_type < std::tuple_size<Coord::tags>::value; }

  template <class Tag> auto get() const {
    // For now we support only variables that are a std::vector. In principle we
    // could support anything that is convertible to gsl::span (or an adequate
    // replacement).
    return gsl::make_span(cast<std::vector<typename Tag::type>>());
  }

  template <class Tag> auto getView() const {
    return cast<DatasetView<Tag>>();
  }

  template <class Tag>
  auto get(std::enable_if_t<std::is_const<Tag>::value> * = nullptr) {
    return const_cast<const Variable *>(this)->get<Tag>();
  }

  template <class Tag>
  auto get(std::enable_if_t<!std::is_const<Tag>::value> * = nullptr) {
    return gsl::make_span(cast<std::vector<typename Tag::type>>());
  }

private:
  template <class T> const T &cast() const;
  template <class T> T &cast();

  uint16_t m_type;
  std::string m_name;
  Unit m_unit;
  cow_ptr<VariableConcept> m_object;
};

template <class Tag, class... Args>
Variable makeVariable(Dimensions dimensions, Args &&... args) {
  return Variable(tag_id<Tag>, Tag::unit, std::move(dimensions),
                  std::vector<typename Tag::type>(std::forward<Args>(args)...));
}

template <class Tag, class T>
Variable makeVariable(Dimensions dimensions, std::initializer_list<T> values) {
  return Variable(tag_id<Tag>, Tag::unit, std::move(dimensions),
                  std::vector<typename Tag::type>(values));
}

Variable operator+(const Variable &a, const Variable &b);
Variable operator*(const Variable &a, const Variable &b);

Variable concatenate(const Dimension dim, const Variable &a1,
                     const Variable &a2);

#endif // VARIABLE_H

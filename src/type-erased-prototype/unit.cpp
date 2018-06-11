#include <stdexcept>

#include <boost/units/systems/si/dimensionless.hpp>
#include <boost/units/systems/si/length.hpp>
#include <boost/units/systems/si/area.hpp>

#include "unit.h"

Unit makeUnit(const boost::units::si::dimensionless &u) {
  return {Unit::Id::Dimensionless};
}
Unit makeUnit(const boost::units::si::length &u) { return {Unit::Id::Length}; }
Unit makeUnit(const boost::units::si::area &u) { return {Unit::Id::Area}; }
template <class T> Unit makeUnit(const T &u) {
  throw std::runtime_error("Unsupported unit combination");
}

Unit operator+(const Unit &a, const Unit &b) {
  if (a != b)
    throw std::runtime_error("Cannot add different units");
  return a;
}

template <class A> Unit multiply(const A &a, const Unit &b) {
  if (b == Unit{Unit::Id::Dimensionless})
    return makeUnit(a * boost::units::si::dimensionless{});
  if (b == Unit{Unit::Id::Length})
    return makeUnit(a * boost::units::si::length{});
  if (b == Unit{Unit::Id::Area})
    return makeUnit(a * boost::units::si::area{});
  throw std::runtime_error("Unsupported unit on RHS");
}

Unit operator*(const Unit &a, const Unit &b) {
  if (a == Unit{Unit::Id::Dimensionless})
    return multiply(boost::units::si::dimensionless{}, b);
  if (a == Unit{Unit::Id::Length})
    return multiply(boost::units::si::length{}, b);
  if (a == Unit{Unit::Id::Area})
    return multiply(boost::units::si::area{}, b);
}
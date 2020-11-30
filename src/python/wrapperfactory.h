#pragma once

#include "external/pybind11/embed.h"
#include <QString>
#include <map>

template<typename GeneralWrappedT, typename GeneralWrapperT> class WrapperFactory
{
private:
  template<typename SpecialWrapperT> static pybind11::object make(GeneralWrappedT& wrapped)
  {
    return pybind11::cast(SpecialWrapperT(wrapped));
  }

public:
  template<typename SpecialWrapperT> static void register_wrapper()
  {
    const auto type = SpecialWrapperT::wrapped_type::TYPE;
    m_creator_map.insert(std::make_pair(type, &make<SpecialWrapperT>));
  }

  static pybind11::object make(GeneralWrappedT& wrapped)
  {
    const auto type = wrapped.type();
    if (m_creator_map.count(type) == 0) {
      return make<GeneralWrapperT>(wrapped);
    } else {
      return m_creator_map.at(type)(wrapped);
    }
  }

  using creator_type = pybind11::object (*)(GeneralWrappedT&);
  using creator_map_type = std::map<QString, creator_type>;

private:
  // NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
  static creator_map_type m_creator_map;
};

template<typename GeneralWrappedT, typename GeneralWrapperT>
typename WrapperFactory<GeneralWrappedT, GeneralWrapperT>::creator_map_type
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
    WrapperFactory<GeneralWrappedT, GeneralWrapperT>::m_creator_map;

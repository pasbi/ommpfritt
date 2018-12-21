#pragma once

#include <pybind11/embed.h>

template<typename GeneralWrappedT, typename GeneralWrapperT>
class WrapperFactory
{
private:
  template<typename SpecialWrapperT> static py::object make(void* wrapped)
  {
    return py::cast(SpecialWrapperT(wrapped));
  }

public:
  template<typename SpecialWrapperT> static void register_wrapper()
  {
    const auto type = SpecialWrapperT::wrapped_type::TYPE;
    m_creator_map.insert(std::make_pair(type, &make<SpecialWrapperT>));
  }

  static py::object make(GeneralWrappedT* wrapped)
  {
    const auto type = wrapped->type();
    if (m_creator_map.count(type) == 0) {
      return make<GeneralWrapperT>(wrapped);
    } else {
      return m_creator_map.at(type)(wrapped);
    }
  }

  using creator_type = py::object(*)(void*);
  using creator_map_type = std::map<std::string, creator_type>;

private:
  static creator_map_type m_creator_map;
};

template<typename GeneralWrappedT, typename GeneralWrapperT>
typename WrapperFactory<GeneralWrappedT, GeneralWrapperT>::creator_map_type
WrapperFactory<GeneralWrappedT, GeneralWrapperT>::m_creator_map;

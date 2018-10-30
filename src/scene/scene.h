#pragma once

#include <map>
#include <memory>
#include <vector>
#include <stdint.h>
#include "python/objectview.h"
#include "external/json_fwd.hpp"
#include <QAbstractItemModel>
#include <unordered_set>

namespace omm
{

class Scene;

template<typename AdapterT> class AdapterRegister
{
public:
  void register_adapter(AdapterT& adapter)
  {
    assert(m_adapters.count(&adapter) == 0);
    m_adapters.insert(&adapter);
  }

  void unregister_adapter(AdapterT& adapter)
  {
    assert(m_adapters.count(&adapter) == 1);
    m_adapters.erase(m_adapters.find(&adapter));
  }

  template<typename F> void for_each(F f)
  {
    std::for_each(m_adapters.begin(), m_adapters.end(), f);
  }

private:
  std::unordered_set<AdapterT*> m_adapters;
};

class AbstractObjectTreeAdapter
{
public:
  virtual void beginInsertObjects(Object& parent, int start, int end) = 0;
  virtual void endInsertObjects() = 0;

  friend class Scene;
};

class AbstractPropertyAdapter
{
public:
  virtual void set_selection(const std::unordered_set<HasProperties*>& selection) = 0;
};

class Object;

class Scene
  : public AdapterRegister<AbstractObjectTreeAdapter>
  , public AdapterRegister<AbstractPropertyAdapter>

{
public:
  Scene();
  ~Scene();

  ObjectView root_view();

  static Scene* currentInstance();
  Object& root() const;

  void reset();
  bool load(const nlohmann::json& data);
  nlohmann::json save() const;

  void insert_object(std::unique_ptr<Object> object, Object& parent);
  void selection_changed();

private:
  std::unique_ptr<Object> m_root;
  static Scene* m_current;
};

}  // namespace omm


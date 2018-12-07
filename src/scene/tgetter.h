#pragma once

#include <map>
#include <memory>
#include <vector>
#include <set>
#include <stdint.h>
#include <QAbstractItemModel>
#include <QUndoStack>

#include "python/objectview.h"
#include "external/json_fwd.hpp"
#include "observed.h"
#include "scene/contextes.h"
#include "scene/abstractselectionobserver.h"
#include "scene/abstractobjecttreeobserver.h"
#include "scene/abstractstylelistobserver.h"
#include "scene/cachedgetter.h"
#include "scene/list.h"

namespace omm
{

class Command;
class Object;
class Project;

}  // namespace omm


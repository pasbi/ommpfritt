#include "registers.h"

namespace omm
{

void register_everything()
{
  register_managers();
  register_objects();
  register_tags();
  register_serializers();
  register_properties();
  register_tools();
  register_nodes();
}

}  // namespace omm

#include "managers/objectmanager/objectmanager.h"
#include "managers/nodemanager/nodemanager.h"
#include "managers/propertymanager/propertymanager.h"
#include "managers/stylemanager/stylemanager.h"
#include "managers/pythonconsole/pythonconsole.h"
#include "managers/historymanager/historymanager.h"
#include "managers/boundingboxmanager/boundingboxmanager.h"
#include "managers/timeline/timeline.h"
#include "managers/curvemanager/curvemanager.h"
#include "managers/dopesheet/dopesheetmanager.h"

void omm::register_managers()
{
  using namespace omm;
#define REGISTER_MANAGER(TYPE) Manager::register_type<TYPE>(#TYPE)

  REGISTER_MANAGER(DopeSheetManager);
  REGISTER_MANAGER(CurveManager);
  REGISTER_MANAGER(ObjectManager);
  REGISTER_MANAGER(PropertyManager);
  REGISTER_MANAGER(StyleManager);
  REGISTER_MANAGER(PythonConsole);
  REGISTER_MANAGER(HistoryManager);
  REGISTER_MANAGER(BoundingBoxManager);
  REGISTER_MANAGER(TimeLine);
  REGISTER_MANAGER(NodeManager);

#undef REGISTER_MANAGER
}

#include "objects/object.h"
#include "objects/ellipse.h"
#include "objects/empty.h"
#include "objects/imageobject.h"
#include "objects/instance.h"
#include "objects/cloner.h"
#include "objects/path.h"
#include "objects/proceduralpath.h"
#include "objects/rectangleobject.h"
#include "objects/view.h"
#include "objects/mirror.h"
#include "objects/text.h"
#include "objects/line.h"
#include "objects/outline.h"
#include "objects/tip.h"


void omm::register_objects()
{
  using namespace omm;
#define REGISTER_OBJECT(TYPE) Object::register_type<TYPE>(#TYPE)
  REGISTER_OBJECT(Cloner);
  REGISTER_OBJECT(Empty);
  REGISTER_OBJECT(Ellipse);
  REGISTER_OBJECT(ImageObject);
  REGISTER_OBJECT(Instance);
  REGISTER_OBJECT(Line);
  REGISTER_OBJECT(Mirror);
  REGISTER_OBJECT(Outline);
  REGISTER_OBJECT(Path);
  REGISTER_OBJECT(ProceduralPath);
  REGISTER_OBJECT(RectangleObject);
  REGISTER_OBJECT(View);
  REGISTER_OBJECT(Text);
  REGISTER_OBJECT(Tip);
#undef REGISTER_OBJECT
}

#include "tags/styletag.h"
#include "tags/nodestag.h"
#include "tags/scripttag.h"
#include "tags/pathtag.h"

void omm::register_tags()
{
  using namespace omm;
#define REGISTER_TAG(TYPE) Tag::register_type<TYPE>(#TYPE);
  REGISTER_TAG(StyleTag);
  REGISTER_TAG(ScriptTag);
  REGISTER_TAG(PathTag);
  REGISTER_TAG(NodesTag);
#undef REGISTER_TAG
}

#include "serializers/abstractserializer.h"
#include "serializers/jsonserializer.h"

void omm::register_serializers()
{
  using namespace omm;
#define REGISTER_DESERIALIZER(TYPE) AbstractDeserializer::register_type<TYPE>(#TYPE);
#define REGISTER_SERIALIZER(TYPE) AbstractSerializer::register_type<TYPE>(#TYPE);

  REGISTER_SERIALIZER(JSONSerializer);
  REGISTER_DESERIALIZER(JSONDeserializer);

#undef REGISTER_SERIALIZER
#undef REGISTER_DESERIALIZER
}

#include "propertywidgets/boolpropertywidget/boolpropertyconfigwidget.h"
#include "propertywidgets/boolpropertywidget/boolpropertywidget.h"
#include "propertywidgets/colorpropertywidget/colorpropertyconfigwidget.h"
#include "propertywidgets/colorpropertywidget/colorpropertywidget.h"
#include "propertywidgets/numericpropertywidget/numericpropertyconfigwidget.h"
#include "propertywidgets/numericpropertywidget/numericpropertywidget.h"
#include "propertywidgets/referencepropertywidget/referencepropertyconfigwidget.h"
#include "propertywidgets/referencepropertywidget/referencepropertywidget.h"
#include "propertywidgets/stringpropertywidget/stringpropertyconfigwidget.h"
#include "propertywidgets/stringpropertywidget/stringpropertywidget.h"
#include "propertywidgets/optionpropertywidget/optionpropertyconfigwidget.h"
#include "propertywidgets/optionpropertywidget/optionpropertywidget.h"
#include "propertywidgets/triggerpropertywidget/triggerpropertyconfigwidget.h"
#include "propertywidgets/triggerpropertywidget/triggerpropertywidget.h"
#include "propertywidgets/vectorpropertywidget/vectorpropertyconfigwidget.h"
#include "propertywidgets/vectorpropertywidget/vectorpropertywidget.h"
#include "propertywidgets/splinepropertywidget/splinepropertyconfigwidget.h"
#include "propertywidgets/splinepropertywidget/splinepropertywidget.h"

void omm::register_properties()
{
  using namespace omm;
#define REGISTER_PROPERTY(TYPE) \
  Property::register_type<TYPE>(#TYPE); \
  Property::m_details[#TYPE] = &TYPE::detail; \
  AbstractPropertyWidget::register_type<TYPE##Widget>(#TYPE"Widget"); \
  AbstractPropertyConfigWidget::register_type<TYPE##ConfigWidget>(#TYPE"ConfigWidget");


  REGISTER_PROPERTY(BoolProperty);
  REGISTER_PROPERTY(ColorProperty);
  REGISTER_PROPERTY(FloatProperty);
  REGISTER_PROPERTY(IntegerProperty);
  REGISTER_PROPERTY(ReferenceProperty);
  REGISTER_PROPERTY(StringProperty);
  REGISTER_PROPERTY(OptionProperty);
  REGISTER_PROPERTY(TriggerProperty);
  REGISTER_PROPERTY(IntegerVectorProperty);
  REGISTER_PROPERTY(FloatVectorProperty);
  REGISTER_PROPERTY(SplineProperty)

#undef REGISTER_PROPERTY
}

#include "tools/tool.h"
#include "tools/brushselecttool.h"
#include "tools/knifetool.h"
#include "tools/pathtool.h"
#include "tools/selectpointstool.h"
#include "tools/selectobjectstool.h"

void omm::register_tools()
{
  using namespace omm;
#define REGISTER_TOOL(TYPE) Tool::register_type<TYPE>(#TYPE);
  REGISTER_TOOL(SelectObjectsTool)
  REGISTER_TOOL(SelectPointsTool)
  REGISTER_TOOL(BrushSelectTool)
  REGISTER_TOOL(KnifeTool)
  REGISTER_TOOL(PathTool)
#undef REGISTER_TOOL
}

#include "nodesystem/nodes/mathnode.h"
#include "nodesystem/nodes/interpolatenode.h"
#include "nodesystem/nodes/spynode.h"
#include "nodesystem/nodes/constantnode.h"
#include "nodesystem/nodes/referencenode.h"
#include "nodesystem/nodes/composenode.h"
#include "nodesystem/nodes/decomposenode.h"
#include "nodesystem/nodes/decomposecolornode.h"
#include "nodesystem/nodes/composecolornode.h"
#include "nodesystem/nodes/fragmentnode.h"
#include "nodesystem/nodes/vertexnode.h"
#include "nodesystem/nodes/colorconvertnode.h"
#include "nodesystem/nodes/functionnode.h"
#include "nodesystem/nodes/function2node.h"
#include "nodesystem/nodes/linepatternnode.h"

void omm::register_nodes()
{
  using namespace omm;
#define REGISTER_NODE(TYPE) Node::register_type<TYPE>(#TYPE); \
  Node::m_details[#TYPE] = &TYPE::detail

  REGISTER_NODE(ReferenceNode);
  REGISTER_NODE(MathNode);
  REGISTER_NODE(SpyNode);
  REGISTER_NODE(ConstantNode);
  REGISTER_NODE(DecomposeNode);
  REGISTER_NODE(ComposeNode);
  REGISTER_NODE(FragmentNode);
  REGISTER_NODE(DecomposeColorNode);
  REGISTER_NODE(ComposeColorNode);
  REGISTER_NODE(VertexNode);
  REGISTER_NODE(ColorConvertNode);
  REGISTER_NODE(FunctionNode);
  REGISTER_NODE(Function2Node);
  REGISTER_NODE(LinePatternNode);
  REGISTER_NODE(InterpolateNode);
#undef REGISTER_NODE
}

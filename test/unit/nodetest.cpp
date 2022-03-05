#include "gtest/gtest.h"
#include "main/application.h"
#include "main/options.h"
#include "nodesystem/nodecompilerglsl.h"
#include "nodesystem/nodecompilerpython.h"
#include "nodesystem/nodemodel.h"
#include "nodesystem/nodes/constantnode.h"
#include "nodesystem/nodes/fragmentnode.h"
#include "nodesystem/nodes/switchnode.h"
#include "nodesystem/nodes/referencenode.h"
#include "nodesystem/ordinaryport.h"
#include "managers/nodemanager/nodeview.h"
#include "properties/colorproperty.h"
#include "properties/floatproperty.h"
#include "registers.h"
#include "renderers/offscreenrenderer.h"
#include "testutil.h"
#include "objects/ellipse.h"
#include <QApplication>

namespace
{

std::unique_ptr<omm::Options> options()
{
  return std::make_unique<omm::Options>(false, // is_cli
                                        false  // have_opengl
  );
}

template<typename Compiler>
class NodeTestFixture
{
public:
  NodeTestFixture()
      : m_omm_app(ommtest::qt_gui_app->gui_application(), options())
      , m_model(omm::nodes::NodeModel(Compiler::LANGUAGE, m_omm_app.scene.get()))
      , m_compiler(m_model)
  {
  }

  omm::nodes::NodeModel& model() { return m_model; }
  const omm::nodes::NodeModel& model() const { return m_model; }
  Compiler& compiler() { return m_compiler; }
  const Compiler& compiler() const { return m_compiler; }
  omm::Scene& scene() const { return *m_omm_app.scene; }

private:
  omm::Application m_omm_app;
  omm::nodes::NodeModel m_model;
  Compiler m_compiler;
};

class GLSLNodeTestFixture : public NodeTestFixture<omm::nodes::NodeCompilerGLSL>
{
public:
  template<typename Property> [[nodiscard]] auto& create_constant_node_output(const QString& name = "")
  {
    auto& node = model().add_node(std::make_unique<omm::nodes::ConstantNode>(model()));
    const auto& property = node.add_property(name, std::make_unique<Property>());
    return *node.find_port<omm::nodes::OutputPort>(property);
  }

  [[nodiscard]] auto& fragment_node() const
  {
    const auto nodes = model().nodes();
    static constexpr auto is_fragment_node = [](const omm::nodes::Node* node) {
      return node->type() == omm::nodes::FragmentNode::TYPE;
    };
    return dynamic_cast<omm::nodes::FragmentNode&>(**std::find_if(nodes.begin(), nodes.end(), is_fragment_node));
  }

  [[nodiscard]] bool compile()
  {
    return m_offscreen_renderer.set_fragment_shader(compiler().code());
  }

private:
  omm::OffscreenRenderer m_offscreen_renderer;
};

class PythonNodeTestFixture : public NodeTestFixture<omm::nodes::NodeCompilerPython>
{
};

}  // namespace

TEST(GLSLNodeTest, empty_model)
{
  SKIP_IF_NO_OPENGL;
  GLSLNodeTestFixture test;
  EXPECT_TRUE(test.compile());
}

TEST(GLSLNodeTest, simple_model)
{
  SKIP_IF_NO_OPENGL;
  GLSLNodeTestFixture test;
  auto& color_property = test.create_constant_node_output<omm::ColorProperty>();
  test.fragment_node().input_port().connect(&color_property);
  EXPECT_TRUE(test.compile());
}

TEST(GLSLNodeTest, incompatible_connection)
{
  SKIP_IF_NO_OPENGL;
  GLSLNodeTestFixture test;
  test.fragment_node().input_port().connect(&test.create_constant_node_output<omm::FloatProperty>());
  EXPECT_FALSE(test.compile());
}

TEST(GLSLNodeTest, switch_node)
{
  SKIP_IF_NO_OPENGL;
  GLSLNodeTestFixture test;

  using omm::nodes::OutputPort;
  using omm::nodes::InputPort;
  auto& switch_node = test.model().add_node(std::make_unique<omm::nodes::SwitchNode>(test.model()));
  test.fragment_node().input_port().connect(switch_node.find_port<OutputPort>(1));
  auto& constant_color_port = test.create_constant_node_output<omm::ColorProperty>();
  for (int i = 0; i < 9; ++i) {
    auto* s_i = switch_node.find_port<InputPort>(i + 1);
    s_i->connect(&constant_color_port);
  }
  EXPECT_TRUE(test.compile());
}

TEST(PythonNodeTest, reference_node)
{
  PythonNodeTestFixture test;
  omm::NodeView node_view;
  node_view.set_model(&test.model());
  auto reference_node = std::make_unique<omm::nodes::ReferenceNode>(test.model());
  auto& ref_node = *reference_node;
  test.model().add_node(std::move(reference_node));

  auto& name_input = ref_node.add_forwarding_port(omm::nodes::PortType::Input, omm::Ellipse::NAME_PROPERTY_KEY);
  auto& name_output = ref_node.add_forwarding_port(omm::nodes::PortType::Output, omm::Ellipse::NAME_PROPERTY_KEY);
  EXPECT_EQ(name_input.data_type(), omm::Type::Invalid);
  EXPECT_EQ(name_output.data_type(), omm::Type::Invalid);

  const auto ellipse = std::make_unique<omm::Ellipse>(&test.scene());
  ref_node.properties().at(omm::nodes::ReferenceNode::REFERENCE_PROPERTY_KEY)->set(ellipse.get());

  EXPECT_EQ(name_input.data_type(), omm::Type::String);
  EXPECT_EQ(name_output.data_type(), omm::Type::String);

  auto* const no_ref =static_cast<omm::AbstractPropertyOwner*>(nullptr);
  ref_node.properties().at(omm::nodes::ReferenceNode::REFERENCE_PROPERTY_KEY)->set(no_ref);

  EXPECT_EQ(name_input.data_type(), omm::Type::Invalid);
  EXPECT_EQ(name_output.data_type(), omm::Type::Invalid);
}

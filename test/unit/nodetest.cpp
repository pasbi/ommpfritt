#include "gtest/gtest.h"
#include "nodesystem/nodecompilerglsl.h"
#include "nodesystem/nodemodel.h"
#include "nodesystem/nodes/constantnode.h"
#include "nodesystem/nodes/fragmentnode.h"
#include "nodesystem/nodes/switchnode.h"
#include "nodesystem/ordinaryport.h"
#include "properties/colorproperty.h"
#include "properties/floatproperty.h"
#include "registers.h"
#include "renderers/offscreenrenderer.h"
#include "testutil.h"
#include <QApplication>

namespace
{

class GLSLNodeTest : public ::testing::Test
{
protected:
  GLSLNodeTest()
      : m_model(omm::nodes::NodeModel(omm::nodes::BackendLanguage::GLSL, nullptr))
      , m_compiler(m_model)
  {
  }

  ommtest::GuiApplication m_application;
  omm::nodes::NodeModel m_model;
  omm::nodes::NodeCompilerGLSL m_compiler;
  omm::OffscreenRenderer m_offscreen_renderer;

  template<typename Property> [[nodiscard]] auto& create_constant_node_output(const QString& name = "")
  {
    auto& node = m_model.add_node(std::make_unique<omm::nodes::ConstantNode>(m_model));
    const auto& property = node.add_property(name, std::make_unique<Property>());
    return *node.find_port<omm::nodes::OutputPort>(property);
  }

  [[nodiscard]] auto& fragment_node() const
  {
    const auto nodes = m_model.nodes();
    static constexpr auto is_fragment_node = [](const omm::nodes::Node* node) {
      return node->type() == omm::nodes::FragmentNode::TYPE;
    };
    return dynamic_cast<omm::nodes::FragmentNode&>(**std::find_if(nodes.begin(), nodes.end(), is_fragment_node));
  }

  [[nodiscard]] bool compile()
  {
    return m_offscreen_renderer.set_fragment_shader(m_compiler.code());
  }
};

}  // namespace

TEST_F(GLSLNodeTest, empty_model)
{
  EXPECT_TRUE(compile());
}

TEST_F(GLSLNodeTest, simple_model)
{
  fragment_node().input_port().connect(&create_constant_node_output<omm::ColorProperty>());
  EXPECT_TRUE(compile());
}

TEST_F(GLSLNodeTest, incompatible_connection)
{
  fragment_node().input_port().connect(&create_constant_node_output<omm::FloatProperty>());
  EXPECT_FALSE(compile());
}

TEST_F(GLSLNodeTest, switch_node)
{
  using omm::nodes::OutputPort;
  using omm::nodes::InputPort;
  auto& switch_node = m_model.add_node(std::make_unique<omm::nodes::SwitchNode>(m_model));
  fragment_node().input_port().connect(switch_node.find_port<OutputPort>(1));
  auto& constant_color_port = create_constant_node_output<omm::ColorProperty>();
  for (int i = 0; i < 9; ++i) {
    auto* s_i = switch_node.find_port<InputPort>(i + 1);
    s_i->connect(&constant_color_port);
  }
  EXPECT_TRUE(compile());
}

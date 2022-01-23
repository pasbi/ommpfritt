#include "gtest/gtest.h"
#include "nodesystem/nodecompilerglsl.h"
#include "nodesystem/nodemodel.h"
#include "nodesystem/nodes/constantnode.h"
#include "nodesystem/nodes/fragmentnode.h"
#include "nodesystem/ordinaryport.h"
#include "properties/colorproperty.h"
#include "properties/floatproperty.h"
#include "registers.h"
#include "renderers/offscreenrenderer.h"

#include <QApplication>

namespace
{

class GuiApplication
{
private:
  int m_application_argc = 0;
  char** m_application_argv = nullptr;
  QGuiApplication m_application;
public:
  explicit GuiApplication()
      : m_application(m_application_argc, m_application_argv)
  {
    omm::register_everything();
  }
};

class GLSLNodeTest : public ::testing::Test
{
protected:
  GLSLNodeTest()
      : m_model(omm::nodes::NodeModel(omm::nodes::BackendLanguage::GLSL, nullptr))
      , m_compiler(m_model)
  {
  }

  GuiApplication m_application;
  omm::nodes::NodeModel m_model;
  omm::nodes::NodeCompilerGLSL m_compiler;
  omm::OffscreenRenderer m_offscreen_renderer;

  omm::nodes::FragmentNode& fragment_node() const
  {
    const auto nodes = m_model.nodes();
    static constexpr auto is_fragment_node = [](const omm::nodes::Node* node) {
      return node->type() == omm::nodes::FragmentNode::TYPE;
    };
    return dynamic_cast<omm::nodes::FragmentNode&>(**std::find_if(nodes.begin(), nodes.end(), is_fragment_node));
  }

  bool compile()
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
  auto& constant_node = m_model.add_node(std::make_unique<omm::nodes::ConstantNode>(m_model));
  const auto& color_property = constant_node.add_property("", std::make_unique<omm::ColorProperty>());
  const auto& constant_node_color_port = constant_node.find_port<omm::nodes::OutputPort>(color_property);
  fragment_node().input_port().connect(constant_node_color_port);
  EXPECT_TRUE(compile());
}

TEST_F(GLSLNodeTest, incompatible_connection)
{
  auto& constant_node = m_model.add_node(std::make_unique<omm::nodes::ConstantNode>(m_model));
  const auto& property = constant_node.add_property("", std::make_unique<omm::FloatProperty>());
  const auto& port = constant_node.find_port<omm::nodes::OutputPort>(property);
  fragment_node().input_port().connect(port);
  EXPECT_FALSE(compile());
}

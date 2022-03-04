#include "gtest/gtest.h"
#include "main/application.h"
#include "main/options.h"
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

std::unique_ptr<omm::Options> options()
{
  return std::make_unique<omm::Options>(false, // is_cli
                                        false  // have_opengl
  );
}

class GLSLNodeTest
{
public:
  GLSLNodeTest()
      : m_omm_app(ommtest::qt_gui_app->gui_application(), options())
      , m_model(omm::nodes::NodeModel(omm::nodes::BackendLanguage::GLSL, nullptr))
      , m_compiler(m_model)
  {
  }

  omm::nodes::NodeModel& model()
  {
    return m_model;
  }

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

  omm::Application m_omm_app;
  omm::nodes::NodeModel m_model;
  omm::nodes::NodeCompilerGLSL m_compiler;
  omm::OffscreenRenderer m_offscreen_renderer;
};

}  // namespace

TEST(GLSLNodeTest, empty_model)
{
  SKIP_IF_NO_OPENGL;
  GLSLNodeTest test;
  EXPECT_TRUE(test.compile());
}

TEST(GLSLNodeTest, simple_model)
{
  SKIP_IF_NO_OPENGL;
  GLSLNodeTest test;
  auto& color_property = test.create_constant_node_output<omm::ColorProperty>();
  test.fragment_node().input_port().connect(&color_property);
  EXPECT_TRUE(test.compile());
}

TEST(GLSLNodeTest, incompatible_connection)
{
  SKIP_IF_NO_OPENGL;
  GLSLNodeTest test;
  test.fragment_node().input_port().connect(&test.create_constant_node_output<omm::FloatProperty>());
  EXPECT_FALSE(test.compile());
}

TEST(GLSLNodeTest, switch_node)
{
  SKIP_IF_NO_OPENGL;
  GLSLNodeTest test;

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

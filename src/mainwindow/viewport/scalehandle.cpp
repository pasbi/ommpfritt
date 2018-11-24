// #include "mainwindow/viewport/scalehandle.h"
// #include "renderers/abstractrenderer.h"
// #include "mainwindow/viewport/handle.h"

// namespace
// {

// void draw_arrow(omm::AbstractRenderer& renderer, const omm::Style& style)
// {
//   constexpr auto n = 200;
//   std::vector<omm::Point> points;
//   points.reserve(n+1);
//   for (size_t i = 0; i <= n; ++i) {
//     const double x = cos(i * 2.0/n * M_PI) * 50;
//     const double y = sin(i * 2.0/n * M_PI) * 50;
//     points.push_back(omm::Point(arma::vec2 { x, y }));
//   }
//   renderer.draw_spline(points, style);
// }

// }  // namespace

// namespace omm
// {

// void ScaleHandle::draw(AbstractRenderer& renderer) const
// {
//   Style style;
//   style.is_pen_active = true;
//   if (is_active()) {
//     style.pen_color = Color::WHITE;
//   } else if (is_hovered()) {
//     style.pen_color = Color(1.0, 0.3, 1.0);
//   } else {
//     style.pen_color = Color(0.8, 0.0, 0.8);
//   }
//   draw_circle(renderer, style);
// }

// bool ScaleHandle::mouse_move(const arma::vec2& delta, const arma::vec2& pos)
// {
//   if (SubHandle::mouse_move(delta, pos)) {
//     handle.transform_objects(ObjectTransformation().scaled(factor));
//     return true;
//   } else {
//     return false;
//   }
// }

// bool ScaleHandle::contains(const arma::vec2& point) const
// {
//   constexpr double eps = 10;
//   const double r = arma::norm(point);
//   return 50 - eps <= r && r <= 50 + eps;
// }

// }  // namespace omm

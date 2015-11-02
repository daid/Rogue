#ifndef LINE_OF_SIGHT_H
#define LINE_OF_SIGHT_H

#include <functional>

bool has_line_of_sight(int x0, int y0, int x1, int y1);
void visit_field_of_view(int x, int y, int distance, std::function<void(int, int)> callback);

#endif//LINE_OF_SIGHT_H

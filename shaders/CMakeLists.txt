add_custom_target(${PROJECT_NAME}_shaders)

include(../cmake/AddShader.cmake)
dreams_add_shader(${PROJECT_NAME}_shaders font_renderer.vert)
dreams_add_shader(${PROJECT_NAME}_shaders font_renderer.geom)
dreams_add_shader(${PROJECT_NAME}_shaders font_renderer.frag)
dreams_add_shader(${PROJECT_NAME}_shaders image_renderer.vert)
dreams_add_shader(${PROJECT_NAME}_shaders image_renderer.frag)

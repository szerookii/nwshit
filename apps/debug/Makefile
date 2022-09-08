apps += Debug::App
app_headers += apps/debug/app.h

app_debug_src = $(addprefix apps/debug/,\
  app.cpp \
  debug_controller.cpp \
  debug_view.cpp \
)

apps_src += $(app_debug_src)

app_images += apps/debug/deboug_icon.png

i18n_files += $(call i18n_with_universal_for, debug/base)

$(eval $(call depends_on_image,apps/debug/app.cpp,apps/debug/debug_icon.png))
apps += Debug::App
app_headers += apps/debug/app.h

app_debug_src = $(addprefix apps/debug/,\
  app.cpp \
  debug_controller.cpp \
  debug_view.cpp \
)

apps_src += $(app_debug_src)

app_images += apps/debug/debug_icon.png

i18n_files += $(addprefix apps/debug/,\
  base.de.i18n\
  base.en.i18n\
  base.es.i18n\
  base.fr.i18n\
  base.pt.i18n\
  base.it.i18n\
  base.nl.i18n\
  base.hu.i18n\
)


$(eval $(call depends_on_image,apps/debug/app.cpp,apps/debug/debug_icon.png))
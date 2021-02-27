import subprocess, os

project = "veg"
author = "sarah"
copyright = "2020-2020, sarah"
extensions = ["sphinx.ext.imgmath", "sphinx.ext.todo", "breathe"]
templates_path = ["_templates"]
exclude_patterns = ["_build"]
html_static_path = []
html_theme = "sphinx_rtd_theme"
cpp_index_common_prefix = ["veg::"]

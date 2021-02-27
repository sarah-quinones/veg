import subprocess, os

project = "veg"
author = "sarah"
copyright = "2020-2020, sarah"
extensions = [
    "sphinx.ext.imgmath",
    "sphinx.ext.todo",
    "sphinx_tabs.tabs",
]
templates_path = ["_templates"]
exclude_patterns = ["_build"]
html_static_path = []
html_theme = "pydata_sphinx_theme"
cpp_index_common_prefix = ["veg::"]

import subprocess, os

project = "veg"
author = "sarah"
copyright = "2020-2021, sarah"
extensions = [
    "sphinx.ext.imgmath",
    "sphinx.ext.todo",
    "sphinx_tabs.tabs",
    "sphinxcontrib.inlinesyntaxhighlight",
]
templates_path = ["_templates"]
exclude_patterns = ["_build"]
html_static_path = []
html_theme = "sphinx_rtd_theme"
cpp_index_common_prefix = ["veg::"]
add_function_parentheses = False

# use language set by highlight directive if no language is set by role
inline_highlight_respect_highlight = False

# use language set by highlight directive if no role is set
inline_highlight_literals = False

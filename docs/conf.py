import subprocess, os

read_the_docs_build = os.environ.get("READTHEDOCS", None) == "True"
if read_the_docs_build:
    subprocess.call(
        "mkdir -p build && cd build && "
        "cmake -DVEG_BUILD_DOCS=ON ../.. && "
        "make doxygen-docs",
        shell=True,
    )
    xmldir = "build/docs/xml"
    breathe_projects = {"veg": "build/docs/xml"}

project = "veg"
author = "sarah"
copyright = "2020-2020, sarah"
extensions = ["sphinx.ext.imgmath", "sphinx.ext.todo", "breathe"]
templates_path = ["_templates"]
exclude_patterns = ["_build"]
html_static_path = []
html_theme = "sphinx_book_theme"
breathe_default_project = "veg"

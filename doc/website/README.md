# Installation

The static html website is built with `mkdocs`. To install `mkdocs` and the theme:

    pip install mkdocs
    pip install git+https://github.com/mkdocs/mkdocs-bootswatch.git

# Usage

For live preview of the website (updated at each file change):

    mkdocs serve

To build the static html website for deployement:

    mkdocs build -d path/to/output/directory

For more information:

    mkdocs -h

# Website structure

- mkdocs.yml contains the website configuration (including menu)
- docs/ contains website sources (in markdown)
- template/ contains theme customization

# Show / hide the table of content

Modify the following code in `template/base.html`:

    {% if page.title in ('Home','Gallery') %}
        <div class="col-md-12" role="main">{% include "content.html" %}</div>
    {% else %}
        <div class="col-md-3">{% include "toc.html" %}</div>
        <div class="col-md-9" role="main">{% include "content.html" %}</div>
    {% endif %}

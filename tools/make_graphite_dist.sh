

# copy_graphite_dist()
#
# Arguments: none
# Variables: $DIST_DIR: where to copy the distribution
#
#  Copies the current graphite distribution into $DIST_DIR
# and cleans-up dummy files.

copy_graphite_dist() {
    rm -fr $DIST_DIR
    mkdir -p $DIST_DIR
    cp -r ../GraphiteThree $DIST_DIR/
    cp $DIST_DIR/GraphiteThree/README.md $DIST_DIR/
    find $DIST_DIR -name ".git*" -prune -exec rm -fr {} \;
    unix2dos $DIST_DIR/README.md
}

# configure_geogram()
#
# Arguments: none
# Variables: $DIST_DIR: where to copy the distribution
#
#  Copies the current geogram distribution into $DIST_DIR
# and cleans-up dummy files.

configure_geogram() {
     cp $DIST_DIR/GraphiteThree/geogram/CMakeOptions.txt.graphite $DIST_DIR/GraphiteThree/geogram/CMakeOptions.txt
}

# create_archives()
#
# Arguments: $1 full path to the directory to be archived
# Variables: none
#
#   Creates $1.tar.gz and $1.zip in the directory
# that contains $1, then deletes $1

create_archives() {
    echo Archiving $1 ...
    (cd `dirname $1`; tar cvfz `basename $1`.tar.gz `basename $1` &> /dev/null)
    (cd `dirname $1`; zip -r `basename $1`.zip `basename $1` &> /dev/null)
    echo Cleaning-up $1
    rm -fr $1
    echo
}

# create_doc_archive()
#
# Arguments: none
# Variables:
#  $DIST_BASE: where all the archives are generated
#  $DIST_DIR: where the distribution was copied
#   Generates the Doxygen documentation and creates a
# .tar.gz archive of it.

create_doc_archive() {
    echo "Generating documentation archive ...."
    echo "   Configuring build tree"
    (cd $DIST_DIR/GraphiteThree; ./configure.sh &> /dev/null)
    echo "   Generating documentation"
    (cd $DIST_DIR/GraphiteThree/build/Linux64-gcc-dynamic-Release; \
     make doc &> /dev/null; \
     cd doc/doc; \
     tar cvfz $DIST_BASE/graphite3_$VERSION-doc.tar.gz html &> /dev/null)
    echo "   Cleaning-up build tree"
    rm -fr $DIST_DIR/GraphiteThree/build
    echo 
}

usage() {
   echo "Usage: make_graphite_dist  <-no-doc>"
}

while [[ $# -gt 0 ]]
do
   arg="$1"
   case $arg in
      -no-doc)
	  NO_DOC=1
	  ;;
      *|-h|/?)
         usage
         exit
      ;;
   esac
   shift
done

VERSION_MAJOR=`cat geogram/CMakeLists.txt | grep 'set(VORPALINE_VERSION_MAJOR' | sed -e 's|[^0-9]||g'`
VERSION_MINOR=`cat geogram/CMakeLists.txt | grep 'set(VORPALINE_VERSION_MINOR' | sed -e 's|[^0-9]||g'`
VERSION_PATCH=`cat geogram/CMakeLists.txt | grep 'set(VORPALINE_VERSION_PATCH' | sed -e 's|[^0-9]||g'`
VERSION=$VERSION_MAJOR.$VERSION_MINOR.$VERSION_PATCH

DIST_BASE=/tmp/GRAPHITE/
DIST_NAME=graphite3_$VERSION
DIST_DIR=$DIST_BASE/$DIST_NAME
GEOGRAM_PACKAGE=/tmp/GEOGRAM/geogram_$VERSION.tar.gz
rm -fr $DIST_BASE
mkdir -p $DIST_BASE


cat <<EOF

******
******
***
*** Generating $DIST_NAME distribution
***
******
******

EOF

copy_graphite_dist
configure_geogram
if [ ! $NO_DOC ]; then
   create_doc_archive
fi
create_archives $DIST_DIR

echo "Archive files generated in " $DIST_BASE ":"
echo "*******************************************"
echo
ls -al $DIST_BASE

cat <<EOF

******
******
***                                    
*** $DIST_NAME distribution generated 
***
******
******

EOF

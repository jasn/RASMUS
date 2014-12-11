#!/bin/bash
mkdir -p tmp
mkdir -p tmp2

if [ ! -d web ]; then
    git clone git@github.com:jasn/RASMUS.git web
    cd web
    git checkout gh-pages
    mkdir -p users_manual/
    mkdir -p relationssproget_rasmus/
    cd ..
fi

cd manuals/users_manual/

latexmk -pdf rasmus-manual.tex
cp rasmus-manual.pdf ../../web/users_manual.pdf
hevea rasmus-manual.tex -o ../../tmp/rasmus-manual.html -moreentities -fix
sed -nre 's/.*imgsrc\{([^}]*)\}.*/\1/p' rasmus-manual.tex |
    while read x
    do
	cp "$x" ../../web/users_manual/
    done

cd ../..

hacha tmp/rasmus-manual.html -o web/users_manual/index.html


cd manuals/relationssproget_rasmus/
#latexmk -pdf rasmus.tex
#cp rasmus-manual.pdf ../../web/users_manual.pdf
hevea rasmus.tex -o ../../tmp2/rasmus.html -moreentities -fix
cd ../..
hacha tmp2/rasmus.html -o web/relationssproget_rasmus/index.html

cd $1/doc
make apidocs
cd ../..
rm -rf web/doc
cp -r $1/doc/doc/html web/doc

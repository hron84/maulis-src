#!/bin/sh

DATE="`date +"%F %T"`"
export GIT_AUTHOR_NAME="Adam Maulis"
export GIT_AUTHOR_EMAIL="maulis@ludens.elte.hu"

git checkout ftp
wget -q -r -np http://maulis.web.elte.hu/src/
find . -type f -iname 'index.html*' -delete
cp -r maulis.web.elte.hu/src/* .
rm -fr maulis.web.elte.hu
git add . 
git commit -m "Autoupdate ${DATE}"
if [ -d git/refs/remotes/origin ]; then
  git push origin ftp
fi

#/bin/bash
git pull develop
git pull master
git checkout develop
git merge -s ours master
git checkout master
git merge develop
git push
git checkout develop
git push
git push origin --tags
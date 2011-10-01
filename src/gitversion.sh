GITTIME="$(date -d "$(git log -1 --format=format:%ci)" "+%Y/%m/%d %H:%M:%S")"
GITHASH="$(git rev-parse -q --short --verify HEAD)"

echo "#ifndef _GITVERSION_H_"
echo "#define _GITVERSION_H_"
echo "#define GITTIME \"$GITTIME\""
echo "#define GITHASH \"$GITHASH\""
echo "#endif"

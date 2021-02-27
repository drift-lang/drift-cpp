for f in $(ls $pwd)
do
    if [ $f == 'run.sh' ]; then
        continue
    fi
    time ../a.out $f
done
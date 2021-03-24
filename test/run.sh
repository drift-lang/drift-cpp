for f in $(ls test)
do
    if [ $f == 'run.sh' ]; then
	continue
    fi
    echo ./test/$f
    time ./drift ./test/$f
done

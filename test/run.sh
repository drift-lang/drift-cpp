for f in $(ls test)
do
    if [ $f == 'run.sh' ]; then
	continue
    fi
    time ./drift ./test/$f
done

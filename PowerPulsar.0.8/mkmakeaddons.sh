#!/bin/sh
for i in sample_addons/*
do
	( cd $i ; make )
done

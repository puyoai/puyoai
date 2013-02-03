all:
	$(MAKE) -C third_party
	$(MAKE) -C util
	$(MAKE) -C core
	$(MAKE) -C cpu
	$(MAKE) -C duel
	#
	# Make succeeded
	#

tests:
	$(MAKE) -C core tests
	$(MAKE) -C duel tests

clean:
	$(MAKE) -C third_party clean
	$(MAKE) -C core clean
	$(MAKE) -C cpu clean
	$(MAKE) -C duel clean


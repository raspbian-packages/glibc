GLIBC_GIT = https://sourceware.org/git/glibc.git
GLIBC_BRANCH = release/$(DEB_VERSION_UPSTREAM)/master
GLIBC_TAG = glibc-$(DEB_VERSION_UPSTREAM)
GLIBC_CHECKOUT = glibc-checkout
DEB_ORIG_UNCOMPRESSED = ../glibc_$(DEB_VERSION_UPSTREAM).orig.tar
DEB_ORIG = $(DEB_ORIG_UNCOMPRESSED).xz
GIT_UPDATES_DIFF = debian/patches/git-updates.diff

# Note: 'git archive' doesn't support https remotes, so 'git clone' is used as a first step

get-orig-source: $(DEB_ORIG)
$(DEB_ORIG):
	dh_testdir
	git clone --bare $(GLIBC_GIT) $(GLIBC_CHECKOUT)
	git archive -v --format=tar --prefix=$(GLIBC_TAG)/ --remote=$(GLIBC_CHECKOUT) -o $(DEB_ORIG_UNCOMPRESSED) $(GLIBC_TAG)
	rm -rf $(GLIBC_CHECKOUT)
	tar --delete $(GLIBC_TAG)/manual -f $(DEB_ORIG_UNCOMPRESSED)
	xz $(DEB_ORIG_UNCOMPRESSED)

update-from-upstream:
	dh_testdir
	git clone --bare $(GLIBC_GIT) $(GLIBC_CHECKOUT)
	echo "GIT update of $(GLIBC_GIT)/$(GLIBC_BRANCH) from $(GLIBC_TAG)" > $(GIT_UPDATES_DIFF)
	echo "" >> $(GIT_UPDATES_DIFF)
	(cd $(GLIBC_CHECKOUT) && git diff --no-renames $(GLIBC_TAG) $(GLIBC_BRANCH) -- . ':!manual') >> $(GIT_UPDATES_DIFF)
	rm -rf $(GLIBC_CHECKOUT)

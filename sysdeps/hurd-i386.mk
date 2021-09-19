# Glibc should really do this ld.so link for us.
define libc_extra_install
mkdir -p debian/tmp-$(curpass)/lib
ln -s ld.so.1 debian/tmp-$(curpass)/lib/ld.so
mkdir -p debian/tmp-$(curpass)/usr/include/$(DEB_HOST_MULTIARCH)/mach
mv debian/tmp-$(curpass)/usr/include/mach/i386 debian/tmp-$(curpass)/usr/include/$(DEB_HOST_MULTIARCH)/mach/
ln -s ../$(DEB_HOST_MULTIARCH)/mach/i386 debian/tmp-$(curpass)/usr/include/mach/i386
endef

# FIXME: We are having runtime issues with ifunc...
# e.g. calling memset from a statically-linked program
#
# http://www.gnu.org/software/hurd/open_issues/ifunc.html
#
# For now we are just disabling ifunc:
export libc_cv_ld_gnu_indirect_function=no

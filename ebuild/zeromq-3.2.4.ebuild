# Copyright 1999-2013 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# $Header: /var/cvsroot/gentoo-x86/net-libs/zeromq/zeromq-3.2.4.ebuild,v 1.1 2013/09/23 15:29:43 qnikst Exp $

EAPI=5

inherit autotools eutils

DESCRIPTION="ZeroMQ is a brokerless kernel"
HOMEPAGE="http://www.zeromq.org/"
SRC_URI="http://download.zeromq.org/${P}.tar.gz"

LICENSE="LGPL-3"
SLOT="0"
KEYWORDS="~amd64 ~arm ~hppa ~ppc ~ppc64 ~x86 ~amd64-linux ~x86-linux ~ppc-macos ~x64-macos ~x86-macos"
IUSE="pgm test static-libs elibc_glibc"

DEPEND="|| ( sys-devel/gcc sys-devel/gcc-apple )
		pgm? (
		  virtual/pkgconfig
		  =net-libs/openpgm-5.1.118
		)
		elibc_glibc? ( sys-apps/util-linux )"
RDEPEND=""

src_prepare() {
	einfo "Removing bundled OpenPGM library"
	sed -i 's/AM_CONFIG_HEADER/AC_CONFIG_HEADERS/g' configure.in || die
	rm -r "${S}"/foreign/openpgm/libpgm* || die
	# apply effective bit of below commit to fix compilation on Darwin
	# https://github.com/zeromq/zeromq3-x/commit/400cbc208a768c4df5039f401dd2688eede6e1ca
	sed -i -e '/strndup/d' tests/test_disconnect_inproc.cpp || die
	eautoreconf
}

src_configure() {
	local myconf
	use pgm && myconf="--with-system-pgm" || myconf="--without-pgm"
	econf \
	  $(use_enable static-libs static) \
	  $myconf
}

src_install() {
	emake DESTDIR="${D}" install || die "emake install failed"
	dodoc NEWS README AUTHORS ChangeLog || die "dodoc failed"
	doman doc/*.[1-9] || die "doman failed"

	# remove useless .la files
	find "${D}" -name '*.la' -delete

	# remove useless .a (only for non static compilation)
	use static-libs || find "${D}" -name '*.a' -delete
}

src_test() {
	emake -j1 check
}

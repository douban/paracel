# Copyright 2014 Douban Inc.

EAPI="5"
AUTOTOOLS_AUTORECONF=1
inherit eutils git-2 autotools

DESCRIPTION="Increment version of msgpack-0.5.8 which support std::unordered_map"
HOMEPAGE="http://msgpack.org/ https://github.com/msgpack/msgpack-c/"
EGIT_REPO_URI="https://github.com/xunzhang/${PN}-c.git"

LICENSE="Apache-2.0"
SLOT="0"
KEYWORDS="amd64 x86"
IUSE="static-libs test"

DEPEND="sys-devel/autoconf"
		#sys-devel/automake"

DOCS=( AUTHORS ChangeLog README )

src_unpack() {
	git-2_src_unpack
}

# S="${WORKDIR}/${PN}-c-master/"

src_prepare() {
	sed -i 's/-O3 //' configure.in || die
	autotools-multilib_src_prepare
}

src_configure() {
	export CC="gcc-4.7.2"
	export CXX="g++-4.7.2"
}

src_compile() {
	./bootstrap
	eautoreconf
	econf
	emake
}

src_install() {
	emake DESTDIR="${D}" install || die "Install failed"
}


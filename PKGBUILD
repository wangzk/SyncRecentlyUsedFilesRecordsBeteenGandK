pkgname=syncrurecords-git
_pkgname=syncrurecords
pkgver=git20130726
pkgrel=1
pkgdesc="Synchronize the Recently Used Files Records between Gnome & KDE"
arch=('i686' 'x86_64')
url="https://github.com/wangzk/SyncRecentlyUsedFilesRecordsBeteenGandK"
license=('LGPL2')
depends=('kdelibs' 'gtk2')
makedepends=('cmake' 'automoc4')
source=("https://github.com/wangzk/SyncRecentlyUsedFilesRecordsBeteenGandK/archive/master.zip")
md5sums=('12a3bf3aa0b98f1467f67d33fe836d7f')

build() {
  cd "$srcdir"
  mkdir build
  cd build
  cmake ../SyncRecentlyUsedFilesRecordsBeteenGandK-master \
    -DQT_QMAKE_EXECUTABLE=qmake-qt4 \
    -DCMAKE_INSTALL_PREFIX=/usr \
    -DCMAKE_BUILD_TYPE=Release
  make
}

package() {
  cd "$srcdir/build"
  mkdir -p "$pkgdir/usr/bin"
  cp "$srcdir/build/RecentlyUsedSync" "$pkgdir/usr/bin"
#make DESTDIR="$pkgdir" install
}

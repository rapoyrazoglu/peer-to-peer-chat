class Peerchat < Formula
  desc "Peer-to-peer decentralized chat application"
  homepage "https://github.com/rapoyrazoglu/peer-to-peer-chat"
  url "https://github.com/rapoyrazoglu/peer-to-peer-chat/archive/refs/tags/v0.0.1.tar.gz"
  sha256 "2f8391ccad53358df34e8132a340c1d7b8d9a2c076493cda40e0249bb63a58ff"
  license "MIT"

  depends_on "cmake" => :build
  depends_on "libsodium"

  def install
    system "cmake", "-B", "build",
                    "-DCMAKE_BUILD_TYPE=Release",
                    "-DPEERCHAT_BUILD_TESTS=OFF",
                    *std_cmake_args
    system "cmake", "--build", "build"
    bin.install "build/peerchat"
  end

  test do
    assert_match "Usage:", shell_output("#{bin}/peerchat --help")
  end
end

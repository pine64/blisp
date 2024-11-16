{
  description = "A very basic flake";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs?ref=nixos-unstable";
  };

  outputs =
    { self, nixpkgs, ... }@inputs:
    let
      systems = [
        "x86_64-linux"
        "aarch64-linux"
      ];
      forEachSystem = nixpkgs.lib.genAttrs systems;
    in
    {
      packages = forEachSystem (
        system:
        let
          pkgs = nixpkgs.legacyPackages.${system};
        in
        with pkgs;
        {
          blisp = callPackage ./blisp.nix { inherit self; };
          default = self.packages.${system}.blisp;
        }
      );

      devShells = forEachSystem (
        system:
        let
          pkgs = nixpkgs.legacyPackages.${system};
        in
        with pkgs;
        {
          default = mkShell {
            name = "blisp-dev";
            nativeBuildInputs = [ self.packages.${system}.default ];
          };
        }
      );
    };
}

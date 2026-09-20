#!/usr/bin/env python3

import os
import zipfile
import sys

def create_valid_simple(output_dir):
    path = os.path.join(output_dir, "valid_simple.zip")
    with zipfile.ZipFile(path, 'w', zipfile.ZIP_DEFLATED) as zf:
        zf.writestr("test.txt", "This is a valid simple mod.")
        zf.writestr("textures/texture.dds", "Fake texture data")
    print(f"Created {path}")

def create_corrupt_magic(output_dir):
    path = os.path.join(output_dir, "corrupt_magic.zip")
    with open(path, 'wb') as f:
        f.write(b'PK\x03\x04')
        f.write(os.urandom(1024))
    print(f"Created {path}")

def create_traversal_malicious(output_dir):
    path = os.path.join(output_dir, "traversal_malicious.zip")
    with zipfile.ZipFile(path, 'w', zipfile.ZIP_DEFLATED) as zf:
        info = zipfile.ZipInfo("../../malicious.txt")
        zf.writestr(info, "This file attempts path traversal.")
        info2 = zipfile.ZipInfo("C:\\Windows\\System32\\malicious.dll")
        zf.writestr(info2, "This file attempts absolute path traversal on Windows.")
        info3 = zipfile.ZipInfo("/etc/passwd")
        zf.writestr(info3, "This file attempts absolute path traversal on Linux.")
    print(f"Created {path}")

def create_oversized_mock(output_dir):
    path = os.path.join(output_dir, "oversized_mock.zip")
    with open(path, 'wb') as f:
        f.write(b'PK\x03\x04')
        f.truncate(15 * 1024 * 1024 * 1024)
    print(f"Created {path}")

def create_nested_archive(output_dir):
    inner_path = os.path.join(output_dir, "inner.zip")
    with zipfile.ZipFile(inner_path, 'w', zipfile.ZIP_DEFLATED) as zf:
        zf.writestr("inner.txt", "I am inside a nested archive.")
    
    outer_path = os.path.join(output_dir, "nested_archive.zip")
    with zipfile.ZipFile(outer_path, 'w', zipfile.ZIP_DEFLATED) as zf:
        zf.write(inner_path, "inner.zip")
    
    os.remove(inner_path)
    print(f"Created {outer_path}")

def main():
    if len(sys.argv) != 2:
        print(f"Usage: {sys.argv[0]} <output_dir>")
        sys.exit(1)
        
    output_dir = sys.argv[1]
    os.makedirs(output_dir, exist_ok=True)
    
    create_valid_simple(output_dir)
    create_corrupt_magic(output_dir)
    create_traversal_malicious(output_dir)
    create_oversized_mock(output_dir)
    create_nested_archive(output_dir)
    
if __name__ == "__main__":
    main()

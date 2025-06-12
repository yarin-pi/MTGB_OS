import subprocess

command = ["mingw32-make"]

def build(file_a, file_b):
    # Read content from file A
    with open(file_a, 'rb') as f_a:
        content_a = f_a.read()

    result = subprocess.run(command, capture_output=True, text=True)
    # Print the output
    print(result.stdout)  # Command output
    print(result.stderr)

    with open(file_a, 'rb') as f_a:
        content_b = f_a.read()

    with open(file_b, 'rb') as f_b:
        first_512_bytes = f_b.read(512)
        kernel = f_b.read(len(content_a))
        rest = f_b.read()

    t = len(content_a) - len(content_b)
    if t > 0:
        rest = b'\x00' * (t) + rest
    elif t < 0:
        rest = rest[-t:]

    # Combine the parts
    new_content = first_512_bytes + content_b + rest
    
    # Write the new content back to file B
    with open(file_b, 'wb') as f_b:
        f_b.write(new_content)
        
# Example usage
build('kernel.bin', 'disk.img')

# Basic requests
```bash
PUT key1 value1     # Store key1 with value1 in ShardKV server
GET key1            # Retrieve value of key1
DELETE key1         # Delete key1 entirely
```

# Basic responses
```bash
OK                  # Operation OK
VALUE value1        # Value retrieved from GET operation
ERROR NotFound      # Key-value not found
```

# Server setup
```bash
JOIN 127.0.0.1:5000     # Register server 127.0.0.1 on port 5000 to the hash ring
LEAVE 127.0.0.1:5000    # Remove server 127.0.0.1 on port 5000 from the hash ring
```

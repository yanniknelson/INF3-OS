/*
 * TAR File-system Driver
 * SKELETON IMPLEMENTATION -- TO BE FILLED IN FOR TASK (4)
 */

/*
 * STUDENT NUMBER: s
 */
#include "tarfs.h"
#include <infos/kernel/log.h>
#include <infos/util/printf.h>

using namespace infos::fs;
using namespace infos::drivers;
using namespace infos::drivers::block;
using namespace infos::kernel;
using namespace infos::util;
using namespace tarfs;

/**
 * TAR files contain header data encoded as octal values in ASCII.  This function
 * converts this terrible representation into a real unsigned integer.
 *
 * You DO NOT need to modify this function.
 *
 * @param data The (null-terminated) ASCII data containing an octal number.
 * @return Returns an unsigned integer number, corresponding to the input data.
 */
static inline unsigned int octal2ui(const char *data)
{
	// Current working value.
	unsigned int value = 0;

	// Length of the input data.
	int len = strlen(data);

	// Starting at i = 1, with a factor of one.
	int i = 1, factor = 1;
	while (i < len)
	{
		// Extract the current character we're working on (backwards from the end).
		char ch = data[len - i];

		// Add the value of the character, multipled by the factor, to
		// the working value.
		value += factor * (ch - '0');

		// Increment the factor by multiplying it by eight.
		factor *= 8;

		// Increment the current character position.
		i++;
	}

	// Return the current working value.
	return value;
}

// The structure that represents the header block present in
// TAR files.  A header block occurs before every file, this
// this structure must EXACTLY match the layout as described
// in the TAR file format description.
namespace tarfs
{
	struct posix_header
	{
		// TO BE FILLED IN
		char name[100];		/*   0 */
		char mode[8];		/* 100 */
		char uid[8];		/* 108 */
		char gid[8];		/* 116 */
		char size[12];		/* 124 */
		char mtime[12];		/* 136 */
		char chksum[8];		/* 148 */
		char typeflag;		/* 156 */
		char linkname[100]; /* 157 */
		char magic[6];		/* 257 */
		char version[2];	/* 263 */
		char uname[32];		/* 265 */
		char gname[32];		/* 297 */
		char devmajor[8];	/* 329 */
		char devminor[8];	/* 337 */
		char prefix[155];	/* 345 */
							/* 500 */
	} __packed;
}

/**
 * Reads the contents of the file into the buffer, from the specified file offset.
 * @param buffer The buffer to read the data into.
 * @param size The size of the buffer, and hence the number of bytes to read.
 * @param off The offset within the file.
 * @return Returns the number of bytes read into the buffer.
 */
int TarFSFile::pread(void *buffer, size_t size, off_t off)
{
	fs_log.messagef(LogLevel::DEBUG, "Reading %d Bytes from file of size %d at offset %d", size, this->size(), off);
	if (off >= this->size())
	{
		fs_log.messagef(LogLevel::DEBUG, "Offset out of range");
		return 0;
	}

	// TO BE FILLED IN

	// buffer is a pointer to the buffer that should receive the data.
	// size is the amount of data to read from the file.
	// off is the zero-based offset within the file to start reading from.

	int block_size = _owner.block_device().block_size();
	int blocks_to_read = ((size) % block_size) ? (size) / block_size + 2 : (size) / block_size + 1;

	fs_log.messagef(LogLevel::DEBUG, "Reading %d blocks starting at %d", blocks_to_read, _file_start_block + off);
	uint8_t *buff = new uint8_t[block_size * blocks_to_read];
	_owner.block_device().read_blocks(buff, _file_start_block + (off / block_size), blocks_to_read);
	int left_over = off % block_size;
	memcpy(buffer, buff + left_over, size);

	delete buff;

	// char b[256];
	// snprintf(b, sizeof(b), "Read Sector ");
	// for (int i = 0; i < size + 5; i++)
	// {
	// 	snprintf(b, sizeof(b), "%s%lx ", b, (buff + left_over)[i]);
	// }
	// fs_log.messagef(LogLevel::DEBUG, "%s", b);

	// char b2[256];
	// snprintf(b2, sizeof(b2), "Output      ");
	// for (int i = 0; i < size; i++)
	// {
	// 	snprintf(b2, sizeof(b2), "%s%lx ", b2, ((uint8_t *)buffer)[i]);
	// }
	// fs_log.messagef(LogLevel::DEBUG, "%s", b2);

	return size;
}

/**
 * Reads all the file headers in the TAR file, and builds an in-memory
 * representation.
 * @return Returns the root TarFSNode that corresponds to the TAR file structure.
 */
TarFSNode *TarFS::build_tree()
{
	// Create the root node.
	TarFSNode *root = new TarFSNode(NULL, "", *this);

	// TO BE FILLED IN

	size_t block_size = block_device().block_size();
	size_t nr_blocks = block_device().block_count();
	fs_log.messagef(LogLevel::DEBUG, "There are %d blocks", nr_blocks);

	// You must read the TAR file, and build a tree of TarFSNodes that represents each file present in the archive.
	int start = 0;
	posix_header head;
	int zero_count = 0;
	while (start < nr_blocks)
	{
		uint8_t *buff = new uint8_t[block_size];
		fs_log.messagef(LogLevel::DEBUG, "Reading from %d", start);
		block_device().read_blocks(buff, start, 1);
		if (is_zero_block(buff))
		{
			fs_log.message(LogLevel::DEBUG, "Zero Block found");
			if (zero_count)
			{
				break;
			}
			zero_count += 1;
			start += 1;
			continue;
		}
		else
		{
			zero_count = 0;
		}
		memcpy(&head, buff, block_size);
		int file_blocks = (octal2ui(head.size) % block_size) ? octal2ui(head.size) / block_size + 1 : octal2ui(head.size) / block_size;
		fs_log.messagef(LogLevel::DEBUG, "Adding File %s, size %d", head.name, octal2ui(head.size));
		TarFSNode *last_node;
		TarFSNode *current_node = root;
		TarFSNode *next_node;
		char *path = head.name;
		char component[256];
		do
		{
			int i = 0;
			while (*path && *path != '/')
			{
				component[i++] = *path++;
			}
			component[i] = 0;
			if (*path == '/')
				path++;

			if (strlen(component) == 0)
			{
				break;
			}

			next_node = (TarFSNode *)current_node->get_child(component);
			if (next_node == NULL)
			{
				next_node = new TarFSNode(current_node, component, *this);
				fs_log.messagef(LogLevel::DEBUG, "Adding component %s to the tree at %s", next_node->name().c_str(), current_node->name().c_str());
				current_node->add_child(component, next_node);
			}
			current_node = next_node;
			last_node = current_node;
		} while (current_node);
		last_node->size(octal2ui(head.size));
		last_node->set_block_offset(start);
		fs_log.messagef(LogLevel::DEBUG, "%s is %d bytes large and is at at block offset %d ", last_node->name().c_str(), last_node->size(), start);
		start += 1 + file_blocks;
	}

	return root;
}

/**
 * Returns the size of this TarFS File
 */
unsigned int TarFSFile::size() const
{
	// TO BE FILLED IN
	return octal2ui(this->_hdr->size);
}

/* --- YOU DO NOT NEED TO CHANGE ANYTHING BELOW THIS LINE --- */

/**
 * Mounts a TARFS filesystem, by pre-building the file system tree in memory.
 * @return Returns the root node of the TARFS filesystem.
 */
PFSNode *TarFS::mount()
{
	// If the root node has not been generated, then build it.
	if (_root_node == NULL)
	{
		_root_node = build_tree();
	}

	// Return the root node.
	return _root_node;
}

/**
 * Constructs a TarFS File object, given the owning file system and the block
 */
TarFSFile::TarFSFile(TarFS &owner, unsigned int file_header_block)
	: _hdr(NULL),
	  _owner(owner),
	  _file_start_block(file_header_block),
	  _cur_pos(0)
{
	// Allocate storage for the header.
	_hdr = (struct posix_header *)new char[_owner.block_device().block_size()];

	fs_log.messagef(LogLevel::DEBUG, "Reading from %d", _file_start_block);
	// Read the header block into the header structure.
	_owner.block_device().read_blocks(_hdr, _file_start_block, 1);

	fs_log.messagef(LogLevel::DEBUG, "TarFSFile %s of with %d bytes created at block offset %d", _hdr->name, octal2ui(_hdr->size), _file_start_block);

	// Increment the starting block for file data.
	_file_start_block++;
}

TarFSFile::~TarFSFile()
{
	// Delete the header structure that was allocated in the constructor.
	delete _hdr;
}

/**
 * Releases any resources associated with this file.
 */
void TarFSFile::close()
{
	// Nothing to release.
}

/**
 * Reads the contents of the file into the buffer, from the current file offset.
 * The current file offset is advanced by the number of bytes read.
 * @param buffer The buffer to read the data into.
 * @param size The size of the buffer, and hence the number of bytes to read.
 * @return Returns the number of bytes read into the buffer.
 */
int TarFSFile::read(void *buffer, size_t size)
{
	// Read can be seen as a special case of pread, that uses an internal
	// current position indicator, so just delegate actual processing to
	// pread, and update internal state accordingly.

	fs_log.messagef(LogLevel::DEBUG, "Trying to read");

	// Perform the read from the current file position.
	int rc = pread(buffer, size, _cur_pos);

	fs_log.messagef(LogLevel::DEBUG, "Finished reading %d bytes", rc);

	// Increment the current file position by the number of bytes that was read.
	// The number of bytes actually read may be less than 'size', so it's important
	// we only advance the current position by the actual number of bytes read.
	_cur_pos += rc;

	// Return the number of bytes read.
	return rc;
}

/**
 * Moves the current file pointer, based on the input arguments.
 * @param offset The offset to move the file pointer either 'to' or 'by', depending
 * on the value of type.
 * @param type The type of movement to make.  An absolute movement moves the
 * current file pointer directly to 'offset'.  A relative movement increments
 * the file pointer by 'offset' amount.
 */
void TarFSFile::seek(off_t offset, SeekType type)
{
	fs_log.messagef(LogLevel::DEBUG, "Seek");
	// If this is an absolute seek, then set the current file position
	// to the given offset (subject to the file size).  There should
	// probably be a way to return an error if the offset was out of bounds.
	if (type == File::SeekAbsolute)
	{
		_cur_pos = offset;
	}
	else if (type == File::SeekRelative)
	{
		_cur_pos += offset;
	}
	if (_cur_pos >= size())
	{
		_cur_pos = size() - 1;
	}
}

TarFSNode::TarFSNode(TarFSNode *parent, const String &name, TarFS &owner) : PFSNode(parent, owner), _name(name), _size(0), _has_block_offset(false), _block_offset(0)
{
}

TarFSNode::~TarFSNode()
{
}

/**
 * Opens this node for file operations.
 * @return 
 */
File *TarFSNode::open()
{
	// This is only a file if it has been associated with a block offset.
	if (!_has_block_offset)
	{
		return NULL;
	}

	// Create a new file object, with a header from this node's block offset.
	return new TarFSFile((TarFS &)owner(), _block_offset);
}

/**
 * Opens this node for directory operations.
 * @return 
 */
Directory *TarFSNode::opendir()
{
	return new TarFSDirectory(*this);
}

/**
 * Attempts to retrieve a child node of the given name.
 * @param name
 * @return 
 */
PFSNode *TarFSNode::get_child(const String &name)
{
	TarFSNode *child;

	// Try to find the given child node in the children map, and return
	// NULL if it wasn't found.
	if (!_children.try_get_value(name.get_hash(), child))
	{
		return NULL;
	}

	return child;
}

/**
 * Creates a subdirectory in this node.  This is a read-only file-system,
 * and so this routine does not need to be implemented.
 * @param name
 * @return 
 */
PFSNode *TarFSNode::mkdir(const String &name)
{
	// DO NOT IMPLEMENT
	return NULL;
}

/**
 * A helper routine that updates this node with the offset of the block
 * that contains the header of the file that this node represents.
 * @param offset The block offset that corresponds to this node.
 */
void TarFSNode::set_block_offset(unsigned int offset)
{
	_has_block_offset = true;
	_block_offset = offset;
}

/**
 * A helper routine that adds a child node to the internal children
 * map of this node.
 * @param name The name of the child node.
 * @param child The actual child node.
 */
void TarFSNode::add_child(const String &name, TarFSNode *child)
{
	_children.add(name.get_hash(), child);
}

TarFSDirectory::TarFSDirectory(TarFSNode &node) : _entries(NULL), _nr_entries(0), _cur_entry(0)
{
	_nr_entries = node.children().count();
	_entries = new DirectoryEntry[_nr_entries];

	int i = 0;
	for (const auto &child : node.children())
	{
		_entries[i].name = child.value->name();
		_entries[i++].size = child.value->size();
	}
}

TarFSDirectory::~TarFSDirectory()
{
	delete _entries;
}

bool TarFSDirectory::read_entry(infos::fs::DirectoryEntry &entry)
{
	if (_cur_entry < _nr_entries)
	{
		entry = _entries[_cur_entry++];
		return true;
	}
	else
	{
		return false;
	}
}

void TarFSDirectory::close()
{
}

static Filesystem *tarfs_create(VirtualFilesystem &vfs, Device *dev)
{
	if (!dev->device_class().is(BlockDevice::BlockDeviceClass))
		return NULL;
	return new TarFS((BlockDevice &)*dev);
}

RegisterFilesystem(tarfs, tarfs_create);

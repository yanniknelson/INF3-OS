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
	//if the the read would spill over the end of the file, change the number of bytes being read to the reach the end of the file and no more
	if (off + size > this->size())
	{
		size -= (off + size - this->size());
	}
	//if the offset is out of range or we aren't reading any data, return 0
	if (off >= this->size() || size == 0)
	{
		return 0;
	}

	// buffer is a pointer to the buffer that should receive the data.
	// size is the amount of data to read from the file.
	// off is the zero-based offset within the file to start reading from.

	//get the size of the blocks
	int const block_size = _owner.block_device().block_size();
	//get the number of blocks to read
	//this is the size of the buffer area + the part of the offset within the first block of interest (+1) divided by the block size rounded up
	int const blocks_to_read = ((size + 1 + off % block_size) % block_size) ? (size + 1 + off % block_size) / block_size + 1 : (size + 1 + off % block_size) / block_size;

	//read the blocks of interest into a buffer able to hold it all
	uint8_t *const block_buffer = new uint8_t[block_size * blocks_to_read];
	_owner.block_device().read_blocks(block_buffer, _file_start_block + (off / block_size), blocks_to_read);
	//coppy the region of interest from those blocks into the output buffer
	memcpy(buffer, block_buffer + off % block_size, size);

	//clean up the block buffer
	delete block_buffer;

	//return the size (number of bytes read)
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
	TarFSNode *const root = new TarFSNode(NULL, "", *this);

	//get the block size and the number of blocks
	size_t const block_size = block_device().block_size();
	size_t const nr_blocks = block_device().block_count();

	// You must read the TAR file, and build a tree of TarFSNodes that represents each file present in the archive.

	//starting at the first block
	int offset = 0;
	posix_header head;
	bool zero_seen = false;
	int file_blocks;
	//while we've not read all of the blocks
	while (offset < nr_blocks)
	{
		//get the block we're pointing at with the start offset, should be a header
		//read it into a header structure
		block_device().read_blocks(&head, offset, 1);
		//if the header is all zero
		if (is_zero_block((uint8_t *)(&head)))
		{
			//if we've just seen a zero block, stop, we're at the end of the archive
			if (zero_seen)
			{
				break;
			}
			//otherwise this is the first (consecutive) zero block we've encountered so mark it, move to the next block and skip the rest of the loop
			zero_seen += true;
			offset += 1;
			continue;
		}
		//if the header isn't all zero, set the zero_seen variable to false
		zero_seen = false;

		//get the number of blocks the file uses (always block aligned so it's simply the ceiled division of the size of the file by the block size)
		file_blocks = (octal2ui(head.size) % block_size) ? octal2ui(head.size) / block_size + 1 : octal2ui(head.size) / block_size;
		//set up last, current and next node pointers
		TarFSNode *last_node;
		//startcing our search at the root node
		TarFSNode *current_node = root;
		TarFSNode *next_node;

		//search for the file being added to the structure in the tree using its name
		char *path = head.name;
		char component[256];
		do
		{
			//get the next compenent
			int i = 0;
			while (*path && *path != '/')
			{
				component[i++] = *path++;
			}
			//ensure the component is a null terminated string
			component[i] = 0;
			//if the path/name has another component, move to the start of it
			if (*path == '/')
				path++;

			//if the component just retrieved is empty the path has been fully traversed
			if (strlen(component) == 0)
			{
				break;
			}

			//get the node corresponding to the next component
			next_node = (TarFSNode *)current_node->get_child(component);
			if (next_node == NULL)
			{
				//if the next node is null then the node doesn't exist in the tree and needs to be created and added
				next_node = new TarFSNode(current_node, component, *this);
				current_node->add_child(component, next_node);
			}
			//move the next node into the current node and the current node node into the last node
			current_node = next_node;
			last_node = current_node;
		} while (current_node);
		//at the end of this loop, last_node will hold the lowest node in the path/name and therefore the node corresponding to the file/directory of interest
		//so add the properties of the file/directory to this node

		//Note: This doesn't care about the order in which the files arrive in the archive, a file within a directory can come before the actual directory information
		//the node for the directory will be created when adding the file but will have no properties and once the directory information has been read, its corresponding node
		//will be given the correct properties

		//I assumed this system doesn't support size limiting but don't see any hard in filling in the size field of the node for a directory for if size limiting is to be implemented in the future
		last_node->size(octal2ui(head.size));

		//check if the node being added is not a directory, if so add the offset, if it is a directory I don't
		//NOTE: I included a commented fix in opendir for being able to read files like directories (using /usr/ls)
		if (head.typeflag != '5')
		{
			last_node->set_block_offset(offset);
		}

		//move the offset to point to the header of the next file/directory
		offset += 1 + file_blocks;
	}
	//by the end of this loop, the entire archive has been read (though not necessarily all of the blocks due to checking for the two consecutive zero blocks indicating the end of the archive)

	return root;
}

/**
 * Returns the size of this TarFS File
 */
unsigned int TarFSFile::size() const
{
	//the size can be retrieved from the header, do so and convert it from an octal string to an unsigned integer
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

	// Read the header block into the header structure.
	_owner.block_device().read_blocks(_hdr, _file_start_block, 1);

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

	// Perform the read from the current file position.
	int rc = pread(buffer, size, _cur_pos);

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
	//I ADDED THIS CHECK TO FIX opendir, without this check you can call /usr/ls on files
	// if (_has_block_offset)
	// {
	// 	return NULL;
	// }
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

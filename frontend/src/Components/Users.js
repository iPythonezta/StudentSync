import React, {useState, useEffect, useRef} from "react";

import { useNavigate } from "react-router-dom";
import { useContextApi } from "../ContexApi";
import { toast } from "react-toastify";
import { Button, Container, Modal, Table, Form } from "react-bootstrap";

import axios from "axios";

import logo from '../Assets/logo.PNG';

export default function Users(){

    const {token, setToken, login, setLogin, userData} = useContextApi();
    const [users, setUsers] = useState([]);
    const [modOpen, setModOpen] = useState(false);
    const [newUser, setNewUser] = useState({name: "", email: "", password: "", role: "user", isAdmin: false});
    const navigate = useNavigate();
    
    const fetchUsers = () => {
        axios.get("http://localhost:2028/api/users/").then((res) => {
            if (res.status == 200) {
                setUsers(res.data.users);
                console.log(res.data.users);
            }
        }).catch((err) => {
            console.log(err);
        })
    }

    const handleAddUser = () => {
        axios.post("http://localhost:2028/api/register/", newUser).then((res) => {
            if (res.status == 200) {
                toast.success("User Added Successfully!");
                setModOpen(false);
                fetchUsers();
            }
        }).catch((err) => {
            console.log(err);
            toast.error(err?.response.data);
        })
    }

    const handleDeleteUser = (email) => {
        axios.delete('http://localhost:2028/api/users/remove-user/', {data: {email: email}}).then((res) => {
            if (res.status == 200) {
                toast.success("User Deleted Successfully!");
                fetchUsers();
            }
        }).catch((err) => {
            toast.error(err?.response.data);
        })
    }

    const handleMakeAdmin = (email) => {
        axios.post('http://localhost:2028/api/users/make-admin/', {email: email}).then((res) => {
            if (res.status == 200) {
                toast.success("User is now an Admin!!");
                fetchUsers();
            }
        }).catch((err) => {
            toast.error(err?.response.data);
        })
    }

    
    useEffect(() => {
        if (!login ||  !userData?.isAdmin) {
            navigate("/login")
        }
        fetchUsers();
    }, [login]);
    
    
    return(
        <div style={{display:'flex', alignItems:'stretch', height:'100%'}}>
            <Container className="sidebar" style={{maxWidth: "100%", backgroundColor: "#2463AB", width: "20%", padding: '0 !important', margin:'0 !important'}}>
                <p className="h2 text-center mt-5" style={{color:'white'}}>Navigation</p>
                <Container className="sidebar-links">
                    <p className="nav-btn text-center" onClick={()=>navigate("/")}>Home</p>
                    {userData?.isAdmin && <p className="nav-btn text-center active" onClick={()=>navigate("/users")}>Users</p>}
                    <p className="nav-btn text-center">Aggregate Calculator</p>
                    <p className="nav-btn text-center">Group Former</p>
                    <p className="nav-btn text-center">Quiz Bank</p>
                    <p className="nav-btn text-center">Your Profile</p>
                    <p className="nav-btn text-center" onClick={() => {setToken(null); localStorage.removeItem("token"); setLogin(false); navigate("/login")}}><strong>Logout</strong></p>
                </Container>
            </Container>
            <Container className="main" style={{padding:'0', margin:'0', minHeight:'100vh'}}>
                <Container className="header" style={{width:'100%', border:'1px solid black', padding: '0', margin:'0', maxHeight:'15%', height:'100px'}}>
                    <Container style={{padding: '20px', textAlign:'center', display:'flex', alignItems:'center', justifyContent:'center'}}>
                        <img src={logo} style={{width:'50px', height:'50px', marginRight:'10px'}} alt="logo" />
                        <p className="h1">StudentSync</p>
                    </Container>
                </Container>
                <Container className="mainContent" style={{backgroundColor:'#D4D4D4', minHeight:'85%',display:'flex',flexDirection:'column'}}>
                    <Container className="text-center">
                        <h2 className="text-center mt-3">Users</h2>
                        <p className="text-muted mt-2">(User Management Dashboard)</p>
                    </Container>
                    <Container className="buttons-container">
                        <Button variant="primary" style={{backgroundColor:"#2463AB"}} onClick={()=>setModOpen(true)}>
                            Add User
                        </Button>
                    </Container>
                    <Container style={{width:'80%', margin:'0 auto', marginTop:'20px'}}>
                        <Table striped bordered hover className="users-table">
                            <thead>
                                <tr>
                                    <th>#</th>
                                    <th>Name</th>
                                    <th>Email/Username</th>
                                    <th>Role</th>
                                    <th>Admin Actions</th>
                                </tr>
                            </thead>
                            <tbody>
                                {users?.map((user, index) => (
                                    <tr key={index}>
                                        <td>{index+1}</td>
                                        <td>{user.name}</td>
                                        <td>{user.email}</td>
                                        <td>{user.isAdmin == 1 ? "Admin" : "User"}</td>
                                        <td>
                                            {user.isAdmin == 0 && <Container className="table-btns">
                                                <Button variant="danger" onClick={()=>handleDeleteUser(user.email)}  size="sm" style={{backgroundColor:"#D9534F", marginRight:'10px'}}>Delete</Button>
                                                <Button variant="warning" onClick={()=>handleMakeAdmin(user.email)} size="sm"  style={{backgroundColor:"#F0AD4E"}}>Make Admin</Button>
                                            </Container>}
                                        </td>
                                    </tr>
                                ))}
                            </tbody>
                        </Table>
                    </Container>
                </Container>
                <Container className="modals-container">
                    <Modal show={modOpen} onHide={()=>setModOpen(false)}>
                        <Modal.Header closeButton>
                            <Modal.Title>Add User</Modal.Title>
                        </Modal.Header>
                        <Modal.Body>
                            <Container>
                                <Form>
                                    <Form.Group className="mb-3">
                                        <Form.Label style={{marginBottom:'-20px'}}>Name</Form.Label>
                                        <Form.Control style={{marginBottom:'10px'}} placeholder="Enter name" o
                                            onChange={(e)=>{
                                                setNewUser({...newUser, name:e.target.value})
                                            }}
                                            value={newUser.name}
                                        />
                                        <Form.Label style={{marginBottom:'-20px'}}>Email</Form.Label>
                                        <Form.Control style={{marginBottom:'10px'}} placeholder="Enter email"
                                            onChange={(e)=>{
                                                setNewUser({...newUser, email:e.target.value})
                                            }}
                                            value={newUser.email}
                                        />
                                        <Form.Label style={{marginBottom:'-20px'}}>Password</Form.Label>
                                        <Form.Control style={{marginBottom:'10px'}} type="password" placeholder="Enter password" 
                                            onChange={(e)=>{
                                                setNewUser({...newUser, password:e.target.value})
                                            }}
                                            value={newUser.password}
                                        />
                                        <Form.Label style={{marginBottom:'-20px'}}>Role</Form.Label>
                                        <Form.Select style={{marginBottom:'10px'}} 
                                            onChange={(e)=>{
                                                setNewUser({...newUser, role:e.target.value, isAdmin:e.target.value == "admin"})
                                            }}
                                            value={newUser.role}
                                        >
                                            <option value="user">User</option>
                                            <option value="admin">Admin</option>
                                        </Form.Select>
                                    </Form.Group>
                                </Form>
                            </Container>
                        </Modal.Body>
                        <Modal.Footer>
                            <Container style={{display:'flex', width:'100%', justifyContent:'flex-end'}}>
                                <Button variant="secondary" onClick={()=>setModOpen(false)}>
                                    Close
                                </Button>
                                <Button variant="primary" style={{backgroundColor:"#2463AB", marginLeft:'10px'}} onClick={handleAddUser}>
                                    Add User
                                </Button>
                            </Container>
                        </Modal.Footer>
                    </Modal>
                </Container>
            </Container>
        </div>
    )
}
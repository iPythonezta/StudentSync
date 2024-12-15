import React, {useState, useEffect, useRef} from "react";

import { useNavigate } from "react-router-dom";
import { useContextApi } from "../ContexApi";
import { toast } from "react-toastify";
import { Button, Container, Modal, Table, Form, Card } from "react-bootstrap";

import axios from "axios";

import logo from '../Assets/logo.PNG';

export default function AggregateCalculatorHome(){

    const {token, setToken, login, setLogin, userData} = useContextApi();
    const [subjects, setSubjects] = useState([]);
    const navigate = useNavigate();

    const fetchSubjects = () => {
        axios.get("http://localhost:2028/api/subjects/").then((res) => {
            if (res.status == 200) {
                setSubjects(res.data.subjects);
            }
        }).catch((err) => {
            console.log(err);
        });
    }

    useEffect(() => {
        if (!login) {
            navigate("/login");
        };
        if (token){
            fetchSubjects();
        }
    }, [token, login])

    return(
        <div style={{display:'flex', alignItems:'stretch', height:'100%'}}>
            <Container className="sidebar" style={{maxWidth: "100%", backgroundColor: "#2463AB", width: "20%", padding: '0 !important', margin:'0 !important'}}>
                <p className="h2 text-center mt-5" style={{color:'white'}}>Navigation</p>
                <Container className="sidebar-links">
                    <p className="nav-btn text-center" onClick={()=>navigate("/")}>Home</p>
                    {userData?.isAdmin && <p className="nav-btn text-center" onClick={()=>navigate("/users")}>Users</p>}
                    <p className="nav-btn text-center active" onClick={()=>navigate("/aggregate-calculator")}>Aggregate Calculator</p>
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
                    <Container className="text-center mt-4">
                        <h2>Your Courses</h2>
                        <p>Please select a course to calculate your aggreagate in.</p>
                    </Container>
                    <Container style={{display:'flex', justifyContent:'center', flexWrap:'wrap'}}>
                        {
                            subjects.map((subject, index) => (
                                <Card style={{ margin:'10px', maxWidth:'500px' }} className="zoomable-card">
                                    <Card.Header>
                                        <Card.Title style={{fontSize:'18px'}}>{subject.name}</Card.Title>
                                        <Card.Subtitle className="mb-2 text-muted">
                                            <strong>
                                                Credit Hours:&nbsp;
                                            </strong>
                                            {subject.credits}
                                        </Card.Subtitle>
                                    </Card.Header>
                                    <Card.Body>
                                        <Card.Text> 
                                            <Container style={{display:'flex', justifyContent:'space-between', flexWrap:'wrap'}}>
                                                <span>
                                                    Quiz Weightage:  {subject.quiz_weightage}&nbsp;
                                                </span>
                                                <span>
                                                    Assignment Weightage: {subject.assignment_weightage}&nbsp;
                                                </span>
                                                <span>
                                                    Final Exams Weightage: {subject.finals_weightage}&nbsp;
                                                </span>
                                                <span>
                                                    Mids Weightage: {subject.mids_weightage}&nbsp;
                                                </span>
                                            </Container>
                                        </Card.Text>
                                    </Card.Body>
                                    <Card.Footer style={{display:'flex', justifyContent:'flex-end'}}>
                                        <Button variant="primary" size="sm">Calculate</Button>
                                    </Card.Footer>
                                </Card>
                            ))
                        }
                    </Container>
                </Container>
            </Container>
        </div>
    )
}